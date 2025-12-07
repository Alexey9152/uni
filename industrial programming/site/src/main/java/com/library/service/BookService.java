package com.library.service;

import com.library.model.Book;
import com.library.model.User;
import com.library.model.UserRole;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

@Service
public class BookService {
    
    @Autowired
    private XmlService xmlService;
    
    private List<Book> books;
    
    public BookService(XmlService xmlService) {
        this.xmlService = xmlService;
        this.books = xmlService.loadBooks();
    }
    
    /**
     * Получить все книги
     */
    public List<Book> getAllBooks() {
        return new ArrayList<>(books);
    }
    
    /**
     * Добавить новую книгу
     */
    public void addBook(Book book) {
        books.add(book);
        saveBooks();
    }
    
    /**
     * Обновить книгу
     */
    public void updateBook(Book book) {
        Optional<Book> existing = books.stream()
                .filter(b -> b.getId().equals(book.getId()))
                .findFirst();
        
        if (existing.isPresent()) {
            Book old = existing.get();
            old.setTitle(book.getTitle());
            old.setAuthors(book.getAuthors());
            old.setYear(book.getYear());
            old.setCategory(book.getCategory());
            old.setPrice(book.getPrice());
            old.setTotal(book.getTotal());
            old.setAvailable(book.getAvailable());
            saveBooks();
        }
    }
    
    /**
     * Получить книгу по ID
     */
    public Optional<Book> getBookById(String id) {
        return books.stream()
                .filter(b -> b.getId().equals(id))
                .findFirst();
    }
    
    /**
     * Изменить цену книги
     */
    public void updatePrice(String bookId, double newPrice) {
        Optional<Book> book = getBookById(bookId);
        book.ifPresent(b -> {
            b.setPrice(newPrice);
            saveBooks();
        });
    }
    
    /**
     * Выдать книгу (уменьшить количество в наличии)
     */
    public void issueBook(String bookId) {
        Optional<Book> book = getBookById(bookId);
        if (book.isPresent() && book.get().getAvailable() > 0) {
            book.get().setAvailable(book.get().getAvailable() - 1);
            saveBooks();
        }
    }
    
    /**
     * Вернуть книгу (увеличить количество в наличии)
     */
    public void returnBook(String bookId) {
        Optional<Book> book = getBookById(bookId);
        if (book.isPresent() && book.get().getAvailable() < book.get().getTotal()) {
            book.get().setAvailable(book.get().getAvailable() + 1);
            saveBooks();
        }
    }
    
    /**
     * Поиск по автору
     */
    public List<Book> searchByAuthor(String author) {
        String query = author.toLowerCase();
        return books.stream()
                .filter(b -> b.getAuthors().toLowerCase().contains(query))
                .collect(Collectors.toList());
    }
    
    /**
     * Поиск по году
     */
    public List<Book> searchByYear(int year) {
        return books.stream()
                .filter(b -> b.getYear() == year)
                .collect(Collectors.toList());
    }
    
    /**
     * Поиск по категории
     */
    public List<Book> searchByCategory(String category) {
        String query = category.toLowerCase();
        return books.stream()
                .filter(b -> b.getCategory().toLowerCase().contains(query))
                .collect(Collectors.toList());
    }
    
    /**
     * Перезагрузить книги из XML
     */
    public void reloadBooks() {
        this.books = xmlService.loadBooks();
    }
    
    private void saveBooks() {
        xmlService.saveBooks(books);
    }
}
