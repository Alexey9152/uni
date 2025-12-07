package com.library.service;

import org.springframework.stereotype.Service;
import java.security.MessageDigest;
import java.security.SecureRandom;
import java.util.Base64;

@Service
public class EncryptionService {
    
    /**
     * Хэширование пароля с солью
     */
    public String hashPassword(String password) {
        try {
            byte[] salt = generateSalt();
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(salt);
            byte[] hashedPassword = md.digest(password.getBytes());
            
            // Объединяем соль и хэш
            byte[] saltAndHash = new byte[salt.length + hashedPassword.length];
            System.arraycopy(salt, 0, saltAndHash, 0, salt.length);
            System.arraycopy(hashedPassword, 0, saltAndHash, salt.length, hashedPassword.length);
            
            return Base64.getEncoder().encodeToString(saltAndHash);
        } catch (Exception e) {
            throw new RuntimeException("Error hashing password", e);
        }
    }
    
    /**
     * Проверка пароля
     */
    public boolean verifyPassword(String password, String hashedPassword) {
        try {
            byte[] decoded = Base64.getDecoder().decode(hashedPassword);
            byte[] salt = new byte[16];
            System.arraycopy(decoded, 0, salt, 0, 16);
            
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            md.update(salt);
            byte[] hashedInput = md.digest(password.getBytes());
            
            byte[] storedHash = new byte[decoded.length - 16];
            System.arraycopy(decoded, 16, storedHash, 0, decoded.length - 16);
            
            return java.util.Arrays.equals(hashedInput, storedHash);
        } catch (Exception e) {
            return false;
        }
    }
    
    /**
     * Генерация случайной соли
     */
    private byte[] generateSalt() {
        SecureRandom random = new SecureRandom();
        byte[] salt = new byte[16];
        random.nextBytes(salt);
        return salt;
    }
}
