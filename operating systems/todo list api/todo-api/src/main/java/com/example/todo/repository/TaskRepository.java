package com.example.todo.repository;

import com.example.todo.model.Task;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.data.jpa.repository.JpaRepository;

import java.util.List;

public interface TaskRepository extends JpaRepository<Task, Long> {

    Page<Task> findByStatus(Task.TaskStatus status, Pageable pageable); // для Page-версии

    List<Task> findByStatus(Task.TaskStatus status); // для GET /tasks без пагинации
}
