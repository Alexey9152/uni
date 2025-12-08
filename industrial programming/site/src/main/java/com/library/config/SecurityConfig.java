package com.library.config;

import com.library.service.UserService;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.security.authentication.dao.DaoAuthenticationProvider;
import org.springframework.security.config.annotation.web.builders.HttpSecurity;
import org.springframework.security.config.annotation.web.configuration.EnableWebSecurity;
import org.springframework.security.core.userdetails.UserDetailsService;
import org.springframework.security.crypto.bcrypt.BCryptPasswordEncoder;
import org.springframework.security.crypto.password.PasswordEncoder;
import org.springframework.security.web.SecurityFilterChain;

@Configuration
@EnableWebSecurity
public class SecurityConfig {

    /**
     * BCrypt encoder для хеширования паролей
     * ВАЖНО: создаётся первым, чтобы избежать циклической зависимости
     */
    @Bean
    public PasswordEncoder passwordEncoder() {
        return new BCryptPasswordEncoder();
    }

    @Bean
    public SecurityFilterChain filterChain(HttpSecurity http) throws Exception {
        http
                .authorizeHttpRequests(auth -> auth
                        // Разрешаем доступ к страницам логина, регистрации и статическим ресурсам
                        .requestMatchers("/", "/login", "/register", "/auth/register", "/error", "/css/**", "/js/**").permitAll()
                        // Все остальные запросы требуют аутентификации
                        .anyRequest().authenticated()
                )
                .formLogin(form -> form
                        .loginPage("/login")                    // Страница логина
                        .loginProcessingUrl("/login")           // URL для обработки POST-запроса логина
                        .usernameParameter("username")          // Имя поля username в форме
                        .passwordParameter("password")          // Имя поля password в форме
                        .defaultSuccessUrl("/dashboard", true)  // Редирект после успешного входа
                        .failureUrl("/login?error")             // Редирект при ошибке логина
                        .permitAll()
                )
                .logout(logout -> logout
                        .logoutUrl("/logout")                   // URL для выхода
                        .logoutSuccessUrl("/login?logout")      // Редирект после выхода
                        .permitAll()
                )
                .csrf(csrf -> csrf.disable());              // Отключаем CSRF для упрощения

        return http.build();
    }

    /**
     * Настройка провайдера аутентификации
     * Используем этот метод вместо прямого внедрения UserService в SecurityConfig
     */
    @Bean
    public DaoAuthenticationProvider authenticationProvider(UserService userService, PasswordEncoder passwordEncoder) {
        DaoAuthenticationProvider authProvider = new DaoAuthenticationProvider();
        authProvider.setUserDetailsService(userService);
        authProvider.setPasswordEncoder(passwordEncoder);
        return authProvider;
    }
}
