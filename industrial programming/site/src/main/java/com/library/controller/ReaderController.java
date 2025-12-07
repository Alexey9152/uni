package com.library.controller;

import com.library.model.Book;
import com.library.model.UserRole;
import com.library.service.BookService;
import com.library.service.UserService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.ResponseEntity;
import org.springframework.security.core.Authentication;
import org.springframework.web.bind.annotation.*;

import java.util.List;
import java.util.Optional;

@RestController
@RequestMapping("/api/reader")
public class ReaderController {
    
    @Autowired
    private BookService bookService;
    
    @Autowired
    private UserService userService;
    
    /**
     * Получить все книги
     */
    @GetMapping("/books")
    public ResponseEntity<List<Book>> getAllBooks(Authentication auth) {
        if (!isReader(auth)) {
            return ResponseEntity.status(403).build();
        }
        return ResponseEntity.ok(bookService.getAllBooks());
    }
    
    /**
     * Поиск по автору
     */
    @GetMapping("/books/search/author")
    public ResponseEntity<List<Book>> searchByAuthor(
            @RequestParam String author,
            Authentication auth) {
        if (!isReader(auth)) {
            return ResponseEntity.status(403).build();
        }
        return ResponseEntity.ok(bookService.searchByAuthor(author));
    }
    
    /**
     * Поиск по году
     */
    @GetMapping("/books/search/year")
    public ResponseEntity<List<Book>> searchByYear(
            @RequestParam int year,
            Authentication auth) {
        if (!isReader(auth)) {
            return ResponseEntity.status(403).build();
        }
        return ResponseEntity.ok(bookService.searchByYear(year));
    }
    
    /**
     * Поиск по категории
     */
    @GetMapping("/books/search/category")
    public ResponseEntity<List<Book>> searchByCategory(
            @RequestParam String category,
            Authentication auth) {
        if (!isReader(auth)) {
            return ResponseEntity.status(403).build();
        }
        return ResponseEntity.ok(bookService.searchByCategory(category));
    }
    
    /**
     * Получить свой аккаунт
     */
    @GetMapping("/account")
    public ResponseEntity<com.library.model.User> getMyAccount(Authentication auth) {
        if (!isReader(auth)) {
            return ResponseEntity.status(403).build();
        }
        
        Optional<com.library.model.User> user = userService.getUserByUsername(auth.getName());
        return user.map(ResponseEntity::ok)
                .orElseGet(() -> ResponseEntity.status(404).build());
    }
    
    /**
     * Получить детали выданных книг
     */
    @GetMapping("/borrowed-books")
    public ResponseEntity<List<Book>> getBorrowedBooks(Authentication auth) {
        if (!isReader(auth)) {
            return ResponseEntity.status(403).build();
        }
        
        Optional<com.library.model.User> user = userService.getUserByUsername(auth.getName());
        if (user.isEmpty()) {
            return ResponseEntity.status(404).build();
        }
        
        List<Book> borrowedBooks = new java.util.ArrayList<>();
        for (String bookId : user.get().getBorrowedBookIds()) {
            Optional<Book> book = bookService.getBookById(bookId);
            book.ifPresent(borrowedBooks::add);
        }
        
        return ResponseEntity.ok(borrowedBooks);
    }
    
    /**
     * Вспомогательный метод для проверки прав
     */
    private boolean isReader(Authentication auth) {
        if (auth == null || !auth.isAuthenticated()) {
            return false;
        }
        
        Optional<com.library.model.User> user = userService.getUserByUsername(auth.getName());
        return user.isPresent() && user.get().getRole() == UserRole.READER;
    }
}
