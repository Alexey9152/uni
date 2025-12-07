package com.library.controller;

import com.library.model.UserRole;
import com.library.service.UserService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.*;
import org.springframework.web.servlet.mvc.support.RedirectAttributes;

@Controller
@RequestMapping("/auth")
public class AuthController {
    
    @Autowired
    private UserService userService;
    
    @PostMapping("/register")
    public String register(
            @RequestParam String username,
            @RequestParam String password,
            @RequestParam String fullName,
            @RequestParam String role,
            RedirectAttributes redirectAttributes) {
        
        UserRole userRole;
        try {
            userRole = UserRole.valueOf(role.toUpperCase());
        } catch (IllegalArgumentException e) {
            redirectAttributes.addFlashAttribute("error", "Некорректная роль");
            return "redirect:/register";
        }
        
        if (userService.register(username, password, fullName, userRole)) {
            redirectAttributes.addFlashAttribute("message", "Регистрация успешна! Пожалуйста, войдите.");
            return "redirect:/login";
        } else {
            redirectAttributes.addFlashAttribute("error", "Пользователь с таким именем уже существует.");
            return "redirect:/register";
        }
    }
}
