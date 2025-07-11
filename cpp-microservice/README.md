# cd freelance_service 
# docker build -t freelance-service . 
# docker run -it --rm -v $(pwd):/app -p 9080:9080 freelance-service



===========================
curl -X POST http://localhost:9080/users/signup \
  -H "Content-Type: application/json" \
  -d '{"tenantId": "deforestation","email": "test@example.com", "password": "mypassword", "name": "Test User", "phone": "1234567890"}'

  curl -X POST http://localhost:9080/users/login \
  -H "Content-Type: application/json" \
  -d '{"email": "test@example.com", "password": "mypassword"}'


  curl "http://localhost:9080/users/auth/me?sessionId=YOUR_SESSION_ID"


  curl "http://localhost:9080/users/session/check?sessionId=YOUR_SESSION_ID"