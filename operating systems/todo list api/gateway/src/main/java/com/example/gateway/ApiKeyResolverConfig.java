package com.example.gateway;

import org.springframework.cloud.gateway.filter.ratelimit.KeyResolver;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import reactor.core.publisher.Mono;

@Configuration
public class ApiKeyResolverConfig {

  @Bean
  public KeyResolver apiKeyResolver() {
    return exchange -> {
      var key = exchange.getRequest().getHeaders().getFirst("X-API-KEY");
      return Mono.just(key != null ? key : "anonymous");
    };
  }
}
