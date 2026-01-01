package com.example.todo.dto;

import jakarta.validation.constraints.NotBlank;
import lombok.*;

@Data
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class TaskDTO {
    private Long id;
    @NotBlank(message = "Title is required")
    private String title;
    private String description;
    private String status;
}
