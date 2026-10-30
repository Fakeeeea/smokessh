package main

import (
        "context"
        "net"
        "io"
        "log"
        "os"
        "os/exec"
        "os/signal"
        "syscall"
        "time"

        "github.com/charmbracelet/ssh"
        "github.com/charmbracelet/wish"
        "github.com/creack/pty"
)

const (
    host = "0.0.0.0";
    port = "2222";
)

func main() {
    s, err := wish.NewServer(
        wish.WithAddress(net.JoinHostPort(host, port)),
        wish.WithHostKeyPath(".ssh/host_ed25519"),

        wish.WithPublicKeyAuth(func(ctx ssh.Context, key ssh.PublicKey) bool {
                                return true;
                        }),

        wish.WithPasswordAuth(func(ctx ssh.Context, password string) bool {
                return true;
            }),

        wish.WithMiddleware(func(next ssh.Handler) ssh.Handler {
            return func(sess ssh.Session) {

                ptyReq, winCh, isPty := sess.Pty()
                if !isPty {
                    io.WriteString(sess, "A PTY is required (try ssh -t ...)\n");
                    sess.Exit(1);
                    return;
                }

                cmd := exec.Command("./smokessh");
                cmd.Env = append(os.Environ(), "TERM="+ptyReq.Term);

                ptmx, err := pty.StartWithSize(cmd, &pty.Winsize{
                    Rows: uint16(ptyReq.Window.Height),
                    Cols: uint16(ptyReq.Window.Width),
                })

                if err != nil {
                    io.WriteString(sess, "Error starting application.\n");
                    return;
                }
                defer ptmx.Close();

                go func() {
                    for win := range winCh {
                        pty.Setsize(ptmx, &pty.Winsize {
                            Rows: uint16(win.Height),
                            Cols: uint16(win.Width),
                        });
                    }
                }()

                go func() {
                    <-sess.Context().Done();
                    cmd.Process.Kill();
                }()

                go io.Copy(ptmx, sess);
                io.Copy(sess,ptmx);
                cmd.Wait();
            }
        }),
    )
    if err != nil {
        log.Fatalln(err);
    }

    log.Println("Starting SSH server on the port 2222...")
    go func() {
        if err := s.ListenAndServe(); err != nil && err != ssh.ErrServerClosed {
            log.Fatalln(err);
        }
    }()

    done := make(chan os.Signal, 1);
    signal.Notify(done, os.Interrupt, syscall.SIGTERM);
    <-done;
    log.Println("Stopping SSH server");
    ctx, cancel := context.WithTimeout(context.Background(), 30*time.Second);
    defer cancel();
    s.Shutdown(ctx);
}