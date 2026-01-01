package com.example.todo.dto;

import lombok.*;

@Data
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class UpdateTaskRequest {
    private String title;
    private String description;
    private String status;
}
