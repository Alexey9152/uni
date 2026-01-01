package com.example.todo.dto;

import jakarta.validation.constraints.NotBlank;
import lombok.*;

@Data
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class CreateTaskRequest {
    @NotBlank(message = "Title cannot be empty")
    private String title;
    private String description;
}
