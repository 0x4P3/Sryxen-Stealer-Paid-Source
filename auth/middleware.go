package auth

import (
	"net/http"
	"time"
)

const (
	AdminUsername = "Admin" // change thiss
	AdminPassword = "Syscall" //  change thiss
)

func AuthMiddleware(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		cookie, err := r.Cookie("session_token")
		if err != nil || cookie.Value != "authenticated" {
			http.Redirect(w, r, "/login", http.StatusSeeOther)
			return
		}
		next.ServeHTTP(w, r)
	})
}

func LoginHandler() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		if r.Method == http.MethodPost {
			username := r.FormValue("username")
			password := r.FormValue("password")

			if username == AdminUsername && password == AdminPassword {
				http.SetCookie(w, &http.Cookie{
					Name:     "session_token",
					Value:    "authenticated",
					Path:     "/",
					HttpOnly: true,
					Expires:  time.Now().Add(24 * time.Hour),
				})
				http.Redirect(w, r, "/admin/dashboard", http.StatusSeeOther)
				return
			}
			http.Error(w, "Invalid credentials", http.StatusUnauthorized)
		} else {
			http.ServeFile(w, r, "web/templates/login.html")
		}
	}
}

func LogoutHandler() http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		http.SetCookie(w, &http.Cookie{
			Name:     "session_token",
			Value:    "",
			Path:     "/",
			HttpOnly: true,
			Expires:  time.Now().Add(-1 * time.Hour),
		})
		http.Redirect(w, r, "/login", http.StatusSeeOther)
	}
}
