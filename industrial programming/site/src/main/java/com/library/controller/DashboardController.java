package com.library.controller;

import com.library.model.UserRole;
import com.library.service.UserService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.security.core.Authentication;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;

import java.util.Optional;

@Controller
public class DashboardController {

    @Autowired
    private UserService userService;

    @GetMapping("/dashboard")
    public String dashboard(Authentication auth) {
        if (auth == null || !auth.isAuthenticated()) {
            return "redirect:/login";
        }

        Optional<com.library.model.User> user = userService.getUserByUsername(auth.getName());
        if (user.isPresent()) {
            if (user.get().getRole() == UserRole.LIBRARIAN) {
                return "redirect:/librarian-dashboard";
            } else {
                return "redirect:/reader-dashboard";
            }
        }
        return "redirect:/login";
    }

    @GetMapping("/reader-dashboard")
    public String readerDashboard(Authentication auth, Model model) {
        if (auth != null && auth.isAuthenticated()) {
            model.addAttribute("username", auth.getName());
        }
        return "reader-dashboard";
    }

    @GetMapping("/librarian-dashboard")
    public String librarianDashboard(Authentication auth, Model model) {
        if (auth != null && auth.isAuthenticated()) {
            model.addAttribute("username", auth.getName());
        }
        return "librarian-dashboard";
    }
    @GetMapping("/thread-monitor")
    public String threadMonitor() {
        return "thread-monitor";
    }

}
