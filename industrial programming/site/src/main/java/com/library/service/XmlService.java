package com.library.service;

import com.library.model.Book;
import com.library.model.User;
import com.library.model.UserRole;
import org.w3c.dom.*;
import org.xml.sax.SAXException;

import javax.xml.XMLConstants;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import javax.xml.validation.Schema;
import javax.xml.validation.SchemaFactory;
import javax.xml.validation.Validator;
import javax.xml.xpath.XPath;
import javax.xml.xpath.XPathFactory;
import org.springframework.stereotype.Service;

import java.io.File;
import java.io.IOException;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;

@Service
public class XmlService {
    
    private static final String LIBRARY_XML = "src/main/resources/data/library.xml";
    private static final String LIBRARY_XSD = "src/main/resources/data/library.xsd";
    private static final String USERS_XML = "src/main/resources/data/users.xml";
    private static final String USERS_XSD = "src/main/resources/data/users.xsd";
    
    /**
     * Загрузить все книги из XML
     */
    public List<Book> loadBooks() {
        List<Book> books = new ArrayList<>();
        try {
            Document doc = parseXml(LIBRARY_XML);
            NodeList bookNodes = doc.getElementsByTagName("book");
            
            for (int i = 0; i < bookNodes.getLength(); i++) {
                Element bookElement = (Element) bookNodes.item(i);
                Book book = parseBookElement(bookElement);
                books.add(book);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return books;
    }
    
    /**
     * Сохранить все книги в XML
     */
    public void saveBooks(List<Book> books) {
        try {
            Document doc = DocumentBuilderFactory.newInstance()
                    .newDocumentBuilder()
                    .newDocument();
            
            Element root = doc.createElement("library");
            doc.appendChild(root);
            
            for (Book book : books) {
                Element bookElement = createBookElement(doc, book);
                root.appendChild(bookElement);
            }
            
            saveXml(doc, LIBRARY_XML);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    /**
     * Загрузить всех пользователей из XML
     */
    public List<User> loadUsers() {
        List<User> users = new ArrayList<>();
        try {
            Document doc = parseXml(USERS_XML);
            NodeList userNodes = doc.getElementsByTagName("user");
            
            for (int i = 0; i < userNodes.getLength(); i++) {
                Element userElement = (Element) userNodes.item(i);
                User user = parseUserElement(userElement);
                users.add(user);
            }
        } catch (Exception e) {
            // Файл может не существовать при первом запуске
        }
        return users;
    }
    
    /**
     * Сохранить всех пользователей в XML
     */
    public void saveUsers(List<User> users) {
        try {
            Document doc = DocumentBuilderFactory.newInstance()
                    .newDocumentBuilder()
                    .newDocument();
            
            Element root = doc.createElement("users");
            doc.appendChild(root);
            
            for (User user : users) {
                Element userElement = createUserElement(doc, user);
                root.appendChild(userElement);
            }
            
            saveXml(doc, USERS_XML);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    /**
     * Валидация XML по XSD схеме
     */
    public boolean validateXml(String xmlPath, String xsdPath) {
        try {
            SchemaFactory factory = SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI);
            Schema schema = factory.newSchema(new File(xsdPath));
            Validator validator = schema.newValidator();
            validator.validate(new javax.xml.transform.stream.StreamSource(new File(xmlPath)));
            return true;
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }
    
    // ============= ПРИВАТНЫЕ МЕТОДЫ =============
    
    private Document parseXml(String xmlPath) throws ParserConfigurationException, IOException, SAXException {
        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        factory.setNamespaceAware(true);
        DocumentBuilder builder = factory.newDocumentBuilder();
        
        File xmlFile = new File(xmlPath);
        if (!xmlFile.exists()) {
            return builder.newDocument();
        }
        
        return builder.parse(xmlFile);
    }
    
    private void saveXml(Document doc, String xmlPath) throws Exception {
        TransformerFactory factory = TransformerFactory.newInstance();
        Transformer transformer = factory.newTransformer();
        transformer.setOutputProperty("indent", "yes");
        transformer.setOutputProperty("{http://xml.apache.org/xslt}indent-amount", "2");
        
        File file = new File(xmlPath);
        file.getParentFile().mkdirs();
        
        DOMSource source = new DOMSource(doc);
        StreamResult result = new StreamResult(file);
        transformer.transform(source, result);
    }
    
    private Book parseBookElement(Element element) {
        Book book = new Book();
        book.setId(element.getAttribute("id"));
        book.setTitle(getElementText(element, "title"));
        
        // Загрузка авторов
        NodeList authorNodes = element.getElementsByTagName("author");
        StringBuilder authors = new StringBuilder();
        for (int i = 0; i < authorNodes.getLength(); i++) {
            if (i > 0) authors.append(", ");
            authors.append(authorNodes.item(i).getTextContent());
        }
        book.setAuthors(authors.toString());
        
        book.setYear(Integer.parseInt(getElementText(element, "year")));
        book.setCategory(getElementText(element, "category"));
        book.setPrice(Double.parseDouble(getElementText(element, "price")));
        book.setTotal(Integer.parseInt(element.getAttribute("total")));
        book.setAvailable(Integer.parseInt(element.getAttribute("available")));
        
        return book;
    }
    
    private Element createBookElement(Document doc, Book book) {
        Element element = doc.createElement("book");
        element.setAttribute("id", book.getId());
        element.setAttribute("total", String.valueOf(book.getTotal()));
        element.setAttribute("available", String.valueOf(book.getAvailable()));
        
        addElement(doc, element, "title", book.getTitle());
        
        // Сохранение авторов
        String[] authorsArray = book.getAuthors().split(",\\s*");
        for (String author : authorsArray) {
            addElement(doc, element, "author", author.trim());
        }
        
        addElement(doc, element, "year", String.valueOf(book.getYear()));
        addElement(doc, element, "category", book.getCategory());
        addElement(doc, element, "price", String.valueOf(book.getPrice()));
        
        return element;
    }
    
    private User parseUserElement(Element element) {
        User user = new User();
        user.setUsername(getElementText(element, "username"));
        user.setPassword(getElementText(element, "password"));
        user.setFullName(getElementText(element, "fullName"));
        user.setRole(UserRole.valueOf(getElementText(element, "role")));
        
        NodeList bookIds = element.getElementsByTagName("borrowedBook");
        for (int i = 0; i < bookIds.getLength(); i++) {
            user.getBorrowedBookIds().add(bookIds.item(i).getTextContent());
        }
        
        return user;
    }
    
    private Element createUserElement(Document doc, User user) {
        Element element = doc.createElement("user");
        addElement(doc, element, "username", user.getUsername());
        addElement(doc, element, "password", user.getPassword());
        addElement(doc, element, "fullName", user.getFullName());
        addElement(doc, element, "role", user.getRole().toString());
        
        Element borrowedBooksElement = doc.createElement("borrowedBooks");
        for (String bookId : user.getBorrowedBookIds()) {
            addElement(doc, borrowedBooksElement, "borrowedBook", bookId);
        }
        element.appendChild(borrowedBooksElement);
        
        return element;
    }
    
    private String getElementText(Element element, String tagName) {
        NodeList nodes = element.getElementsByTagName(tagName);
        if (nodes.getLength() > 0) {
            return nodes.item(0).getTextContent();
        }
        return "";
    }
    
    private void addElement(Document doc, Element parent, String tagName, String value) {
        Element element = doc.createElement(tagName);
        element.setTextContent(value);
        parent.appendChild(element);
    }
}
