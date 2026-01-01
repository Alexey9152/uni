package com.example.todo.util;

import com.example.todo.dto.TaskDTO;
import com.example.todo.model.Task;
import org.springframework.stereotype.Component;

@Component
public class TaskMapper {

    public TaskDTO toDTO(Task task) {
        if (task == null) {
            return null;
        }

        return TaskDTO.builder()
                .id(task.getId())
                .title(task.getTitle())
                .description(task.getDescription())
                .status(task.getStatus() != null ? task.getStatus().toString() : null)
                .build();
    }

    public Task toEntity(TaskDTO dto) {
        if (dto == null) {
            return null;
        }

        return Task.builder()
                .id(dto.getId())
                .title(dto.getTitle())
                .description(dto.getDescription())
                .status(dto.getStatus() != null ? Task.TaskStatus.valueOf(dto.getStatus()) : null)
                .build();
    }
}
