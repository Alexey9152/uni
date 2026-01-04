FROM maven:3.9-eclipse-temurin-17 AS builder
WORKDIR /workspace

COPY pom.xml .
COPY todo-api/pom.xml todo-api/pom.xml
COPY events-worker/pom.xml events-worker/pom.xml
COPY gateway/pom.xml gateway/pom.xml

RUN mvn -q -DskipTests -pl :gateway -am dependency:go-offline

COPY gateway/src gateway/src
RUN mvn -q -DskipTests -pl :gateway -am package

FROM eclipse-temurin:17-jre-alpine
RUN addgroup -g 1001 -S appgroup && adduser -u 1001 -S appuser -G appgroup
RUN apk add --no-cache wget
WORKDIR /app
COPY --from=builder /workspace/gateway/target/*.jar app.jar
RUN chown -R appuser:appgroup /app
USER appuser
EXPOSE 8080
HEALTHCHECK --interval=30s --timeout=10s --start-period=20s --retries=3 \
  CMD wget -nv -t1 --spider http://localhost:8080/actuator/health || exit 1
ENTRYPOINT ["java","-jar","app.jar"]
