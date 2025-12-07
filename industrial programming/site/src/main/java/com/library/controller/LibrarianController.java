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
@RequestMapping("/api/librarian")
public class LibrarianController {
    
    @Autowired
    private BookService bookService;
    
    @Autowired
    private UserService userService;
    
    /**
     * Получить все книги
     */
    @GetMapping("/books")
    public ResponseEntity<List<Book>> getAllBooks(Authentication auth) {
        if (!isLibrarian(auth)) {
            return ResponseEntity.status(403).build();
        }
        return ResponseEntity.ok(bookService.getAllBooks());
    }
    
    /**
     * Добавить новую книгу
     */
    @PostMapping("/books")
    public ResponseEntity<String> addBook(@RequestBody Book book, Authentication auth) {
        if (!isLibrarian(auth)) {
            return ResponseEntity.status(403).build();
        }
        try {
            bookService.addBook(book);
            return ResponseEntity.ok("Книга добавлена успешно");
        } catch (Exception e) {
            return ResponseEntity.status(400).body("Ошибка при добавлении: " + e.getMessage());
        }
    }
    
    /**
     * Изменить цену книги
     */
    @PutMapping("/books/{id}/price")
    public ResponseEntity<String> updatePrice(
            @PathVariable String id,
            @RequestParam double price,
            Authentication auth) {
        if (!isLibrarian(auth)) {
            return ResponseEntity.status(403).build();
        }
        try {
            bookService.updatePrice(id, price);
            return ResponseEntity.ok("Цена обновлена");
        } catch (Exception e) {
            return ResponseEntity.status(400).body("Ошибка: " + e.getMessage());
        }
    }
    
    /**
     * Выдать книгу читателю
     */
    @PostMapping("/books/{bookId}/issue")
    public ResponseEntity<String> issueBook(
            @PathVariable String bookId,
            @RequestParam String readerUsername,
            Authentication auth) {
        if (!isLibrarian(auth)) {
            return ResponseEntity.status(403).build();
        }
        
        try {
            Optional<Book> book = bookService.getBookById(bookId);
            if (book.isEmpty()) {
                return ResponseEntity.status(404).body("Книга не найдена");
            }
            if (book.get().getAvailable() <= 0) {
                return ResponseEntity.status(400).body("Нет доступных экземпляров");
            }
            
            Optional<com.library.model.User> reader = userService.getUserByUsername(readerUsername);
            if (reader.isEmpty()) {
                return ResponseEntity.status(404).body("Читатель не найден");
            }
            
            bookService.issueBook(bookId);
            userService.addBorrowedBook(readerUsername, bookId);
            
            return ResponseEntity.ok("Книга выдана успешно");
        } catch (Exception e) {
            return ResponseEntity.status(400).body("Ошибка: " + e.getMessage());
        }
    }
    
    /**
     * Получить список всех читателей
     */
    @GetMapping("/readers")
    public ResponseEntity<List<com.library.model.User>> getAllReaders(Authentication auth) {
        if (!isLibrarian(auth)) {
            return ResponseEntity.status(403).build();
        }
        return ResponseEntity.ok(userService.getAllReaders());
    }
    
    /**
     * Получить детали читателя
     */
    @GetMapping("/readers/{username}")
    public ResponseEntity<com.library.model.User> getReaderDetails(
            @PathVariable String username,
            Authentication auth) {
        if (!isLibrarian(auth)) {
            return ResponseEntity.status(403).build();
        }
        
        Optional<com.library.model.User> reader = userService.getUserByUsername(username);
        return reader.map(ResponseEntity::ok)
                .orElseGet(() -> ResponseEntity.status(404).build());
    }
    
    /**
     * Вспомогательный метод для проверки прав
     */
    private boolean isLibrarian(Authentication auth) {
        if (auth == null || !auth.isAuthenticated()) {
            return false;
        }
        
        Optional<com.library.model.User> user = userService.getUserByUsername(auth.getName());
        return user.isPresent() && user.get().getRole() == UserRole.LIBRARIAN;
    }
}
