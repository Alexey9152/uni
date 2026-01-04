package com.example.gateway.controller;

import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.Map;

@RestController
@RequestMapping("/fallback")
public class FallbackController {

    @GetMapping("/todo-api")
    public ResponseEntity<Map<String, Object>> todoApiFallback() {
        return ResponseEntity.status(503).body(Map.of(
                "service", "todo-api",
                "status", "unavailable"
        ));
    }
}
