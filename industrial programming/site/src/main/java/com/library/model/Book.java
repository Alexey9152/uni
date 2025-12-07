package com.library.model;

public class Book {
    private String id;
    private String title;
    private String authors;
    private int year;
    private String category;
    private double price;
    private int total;
    private int available;

    public Book() {}

    public Book(String id, String title, String authors, int year, String category, double price, int total, int available) {
        this.id = id;
        this.title = title;
        this.authors = authors;
        this.year = year;
        this.category = category;
        this.price = price;
        this.total = total;
        this.available = available;
    }

    // Getters
    public String getId() { return id; }
    public String getTitle() { return title; }
    public String getAuthors() { return authors; }
    public int getYear() { return year; }
    public String getCategory() { return category; }
    public double getPrice() { return price; }
    public int getTotal() { return total; }
    public int getAvailable() { return available; }

    // Setters
    public void setId(String id) { this.id = id; }
    public void setTitle(String title) { this.title = title; }
    public void setAuthors(String authors) { this.authors = authors; }
    public void setYear(int year) { this.year = year; }
    public void setCategory(String category) { this.category = category; }
    public void setPrice(double price) { this.price = price; }
    public void setTotal(int total) { this.total = total; }
    public void setAvailable(int available) { this.available = available; }
}
