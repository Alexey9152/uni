package com.library.service;

import com.library.model.User;
import com.library.model.UserRole;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.security.core.userdetails.UserDetailsService;
import org.springframework.security.core.userdetails.UsernameNotFoundException;
import org.springframework.security.core.authority.SimpleGrantedAuthority;
import org.springframework.security.crypto.password.PasswordEncoder;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

@Service
public class UserService implements UserDetailsService {

    private final XmlService xmlService;
    private final PasswordEncoder passwordEncoder;
    private List<User> users;

    // Конструктор с параметрами - Spring автоматически инжектит зависимости
    public UserService(XmlService xmlService, PasswordEncoder passwordEncoder) {
        this.xmlService = xmlService;
        this.passwordEncoder = passwordEncoder;
        this.users = xmlService.loadUsers();
    }

    /**
     * Метод для Spring Security - загрузка пользователя по username
     */
    @Override
    public UserDetails loadUserByUsername(String username) throws UsernameNotFoundException {
        Optional<User> userOpt = getUserByUsername(username);
        if (userOpt.isEmpty()) {
            throw new UsernameNotFoundException("Пользователь не найден: " + username);
        }

        User user = userOpt.get();
        String roleName = "ROLE_" + user.getRole().name(); // ROLE_LIBRARIAN или ROLE_READER

        return org.springframework.security.core.userdetails.User.builder()
                .username(user.getUsername())
                .password(user.getPassword()) // Пароль уже захеширован
                .authorities(new SimpleGrantedAuthority(roleName))
                .build();
    }

    /**
     * Регистрация нового пользователя
     */
    public boolean register(String username, String password, String fullName, UserRole role) {
        // Проверка, не существует ли уже пользователь с таким username
        if (users.stream().anyMatch(u -> u.getUsername().equals(username))) {
            return false;
        }

        User user = new User();
        user.setUsername(username);
        user.setPassword(passwordEncoder.encode(password)); // Используем BCrypt
        user.setFullName(fullName);
        user.setRole(role);
        user.setBorrowedBookIds(new ArrayList<>());
        users.add(user);
        saveUsers();
        return true;
    }

    /**
     * Получить пользователя по username
     */
    public Optional<User> getUserByUsername(String username) {
        return users.stream()
                .filter(u -> u.getUsername().equals(username))
                .findFirst();
    }

    /**
     * Получить всех пользователей (для библиотекаря)
     */
    public List<User> getAllUsers() {
        return new ArrayList<>(users);
    }

    /**
     * Получить всех читателей
     */
    public List<User> getAllReaders() {
        List<User> readers = new ArrayList<>();
        for (User user : users) {
            if (user.getRole() == UserRole.READER) {
                readers.add(user);
            }
        }
        return readers;
    }

    /**
     * Добавить выданную книгу в аккаунт читателя
     */
    public void addBorrowedBook(String readerUsername, String bookId) {
        Optional<User> user = getUserByUsername(readerUsername);
        if (user.isPresent()) {
            user.get().getBorrowedBookIds().add(bookId);
            saveUsers();
        }
    }

    /**
     * Вернуть книгу (удалить из списка выданных)
     */
    public void returnBorrowedBook(String readerUsername, String bookId) {
        Optional<User> user = getUserByUsername(readerUsername);
        if (user.isPresent()) {
            user.get().getBorrowedBookIds().remove(bookId);
            saveUsers();
        }
    }

    /**
     * Перезагрузить пользователей из XML
     */
    public void reloadUsers() {
        this.users = xmlService.loadUsers();
    }

    private void saveUsers() {
        xmlService.saveUsers(users);
    }
}
