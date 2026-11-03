package main

import (
    "flag"
    "context"
    "net"
    "io"
    "log"
    "os"
    "os/exec"
    "os/signal"
    "path/filepath"
    "syscall"
    "sync"
    "time"
    "errors"

    "github.com/charmbracelet/ssh"
    "github.com/charmbracelet/wish"
    "github.com/charmbracelet/wish/activeterm"
    "github.com/creack/pty"
)

const (
    port = "2222";
    hostKey = "/keys/host_ed25519"
)

func main() {

    ipVersion := flag.String("ip", "4", "IP version to listen on: 4 or 6");
    presetsDir := flag.String("presets", "./presets", "presets directory");
    exe := flag.String("exe", "./smokessh", "path and name of the executable to run");
    flag.Parse();

    var network, host string
    switch *ipVersion {
    case "4":
        network, host = "tcp4", "0.0.0.0"
    case "6":
        network, host = "tcp6", "::"
    default:
        log.Fatalf("-ip must be 4 or 6, got %q", *ipVersion)
    }

    exePath, err := filepath.Abs(*exe)
    if err != nil {
        log.Fatalln(err)
    }
    switch info, err := os.Stat(exePath); {
    case err != nil:
        log.Fatalf("cannot use executable: %v", err)
    case info.IsDir():
        log.Fatalf("executable path is a directory, not a file: %s", exePath)
    case info.Mode()&0o111 == 0:
        log.Fatalf("file is not executable: %s (mode %v)", exePath, info.Mode())
    }

    presetsPath, err := filepath.Abs(*presetsDir)
    if err != nil {
        log.Fatalln(err)
    }
    switch info, err := os.Stat(presetsPath); {
    case err != nil:
        log.Fatalf("cannot use presets directory: %v", err)
    case !info.IsDir():
        log.Fatalf("presets path is not a directory: %s", presetsPath)
    }

    presetsRel, err := filepath.Rel(filepath.Dir(exePath), presetsPath)
        if err != nil {
            log.Fatalf("cannot make presets path relative to the executable: %v", err)
        }

    addr := net.JoinHostPort(host, port);

    s, err := wish.NewServer(
        wish.WithAddress(addr),
        wish.WithHostKeyPath(hostKey),
        wish.WithPublicKeyAuth(func(ssh.Context, ssh.PublicKey) bool { return true; }),
        wish.WithPasswordAuth(func(ssh.Context, string) bool { return true; }),
        wish.WithMiddleware(runApp(exePath, presetsRel), activeterm.Middleware()),
    )

    if err != nil {
        log.Fatalln(err)
    }

    s.ConnCallback = func(_ ssh.Context, conn net.Conn) net.Conn {
   		return newLoggedConn(conn)
   	}

    ln, err := net.Listen(network, addr)
    if err != nil {
        log.Fatalln(err)
    }

    log.Printf("listening on %s (%s) exe =%s presets=%s", addr, network, exePath, presetsPath)
    go func() {
        if err := s.Serve(ln); err != nil && !errors.Is(err, ssh.ErrServerClosed) {
            log.Fatalln(err)
        }
    }()

    done := make(chan os.Signal, 1)
    signal.Notify(done, os.Interrupt, syscall.SIGTERM)
    <-done

    log.Println("stopping server")
    ctx, cancel := context.WithTimeout(context.Background(), 30*time.Second)
    defer cancel()
    if err := s.Shutdown(ctx); err != nil {
        log.Println("shutdown:", err)
    }
}

type loggedConn struct {
    net.Conn
    ip string
    start time.Time
    once sync.Once
}

func newLoggedConn(c net.Conn) net.Conn {
    ip, _, err := net.SplitHostPort(c.RemoteAddr().String())
    if err != nil {
        ip = c.RemoteAddr().String()
    }
    log.Printf("Connected: %s", ip)
    return &loggedConn{Conn: c, ip: ip, start: time.Now()}
}

func (c *loggedConn) Close() error {
    c.once.Do(func() {
        log.Printf("disconnected: %s (after %s)", c.ip, time.Since(c.start).Round(time.Second))
    })
    return c.Conn.Close()
}

func runApp(exePath, presetsPath string) wish.Middleware {
    return func(next ssh.Handler) ssh.Handler {
        return func(sess ssh.Session) {
            ptyReq, winCh, _ := sess.Pty()

            cmd := exec.CommandContext(sess.Context(), exePath, "-d", presetsPath)
            cmd.Dir = filepath.Dir(exePath)
            cmd.Env = append(os.Environ(), "TERM="+ptyReq.Term)

            ptmx, err := pty.StartWithSize(cmd, &pty.Winsize{
                Rows: uint16(ptyReq.Window.Height),
                Cols: uint16(ptyReq.Window.Width),
            })

            if err != nil {
                log.Printf("failed to start %s: %v", exePath, err)
                wish.Fatalln(sess, "Error starting application.")
                return
            }
            defer ptmx.Close()

            go func() {
                for win := range winCh {
                    _ = pty.Setsize(ptmx, &pty.Winsize{
                        Rows: uint16(win.Height),
                        Cols: uint16(win.Width),
                    })
                }
            }()

            go io.Copy(ptmx, sess)
            io.Copy(sess, ptmx)

            code := 0
            if err := cmd.Wait(); err != nil {
                var ee *exec.ExitError
                if errors.As(err, &ee) {
                    code = ee.ExitCode()
                } else {
                    code = 1
                }
            }

            _ = sess.Exit(code)
        }
    }
}