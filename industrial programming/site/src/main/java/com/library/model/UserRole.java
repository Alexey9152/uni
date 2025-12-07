package com.library.model;

public enum UserRole {
    LIBRARIAN("Библиотекарь"),
    READER("Читатель");

    private final String displayName;

    UserRole(String displayName) {
        this.displayName = displayName;
    }

    public String getDisplayName() {
        return displayName;
    }
}
