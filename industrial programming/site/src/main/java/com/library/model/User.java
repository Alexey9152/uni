package com.library.model;

import java.util.ArrayList;
import java.util.List;

public class User {
    private String username;
    private String password;
    private UserRole role;
    private String fullName;
    private List<String> borrowedBookIds = new ArrayList<>();

    public User() {}

    public User(String username, String password, UserRole role, String fullName) {
        this.username = username;
        this.password = password;
        this.role = role;
        this.fullName = fullName;
    }

    // Getters
    public String getUsername() { return username; }
    public String getPassword() { return password; }
    public UserRole getRole() { return role; }
    public String getFullName() { return fullName; }
    public List<String> getBorrowedBookIds() { return borrowedBookIds; }

    // Setters
    public void setUsername(String username) { this.username = username; }
    public void setPassword(String password) { this.password = password; }
    public void setRole(UserRole role) { this.role = role; }
    public void setFullName(String fullName) { this.fullName = fullName; }
    public void setBorrowedBookIds(List<String> borrowedBookIds) { this.borrowedBookIds = borrowedBookIds; }
}
