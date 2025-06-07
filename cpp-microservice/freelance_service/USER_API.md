# User API Documentation

## Endpoints

### Create User
```bash
curl -X POST http://localhost:9080/users \
  -H "Content-Type: application/json" \
  -d '{
    "name": "John Doe",
    "email": "john@example.com",
    "bio": "Full-stack developer with 5 years of experience",
    "title": "Senior Software Engineer",
    "location": "New York, USA",
    "skills": "Python, JavaScript, React, Node.js",
    "hourlyRate": "50",
    "availability": "Full-time",
    "experience": "5 years",
    "education": "BS in Computer Science",
    "certifications": "AWS Certified Developer",
    "githubUrl": "https://github.com/johndoe",
    "linkedinUrl": "https://linkedin.com/in/johndoe",
    "portfolioUrl": "https://johndoe.dev",
    "profilePicture": "https://example.com/profile.jpg",
    "projects": [
      {
        "name": "E-commerce Platform",
        "description": "Built a full-stack e-commerce platform",
        "techStack": "React, Node.js, MongoDB",
        "role": "Lead Developer",
        "startDate": "2022-01",
        "endDate": "2022-12",
        "projectUrl": "https://github.com/johndoe/ecommerce",
        "isCurrent": false
      }
    ]
  }'
```

### Update User
```bash
curl -X PUT http://localhost:9080/users/1 \
  -H "Content-Type: application/json" \
  -d '{
    "bio": "Updated bio",
    "title": "Lead Developer",
    "location": "San Francisco, USA",
    "skills": "Python, JavaScript, React, Node.js, Go",
    "hourlyRate": "75",
    "availability": "Part-time",
    "experience": "6 years",
    "education": "MS in Computer Science",
    "certifications": "AWS Certified Developer, Google Cloud Professional",
    "githubUrl": "https://github.com/johndoe",
    "linkedinUrl": "https://linkedin.com/in/johndoe",
    "portfolioUrl": "https://johndoe.dev",
    "profilePicture": "https://example.com/new-profile.jpg"
  }'
```

### Get All Users
```bash
curl -X GET http://localhost:9080/users
```

### Get Paginated Users
```bash
curl -X GET "http://localhost:9080/users/list?skip=0&take=10"
```

### Search Users
```bash
curl -X GET "http://localhost:9080/users/search?q=python"
```

### Add Project to User
```bash
curl -X POST http://localhost:9080/users/john@example.com/projects \
  -H "Content-Type: application/json" \
  -d '{
    "name": "AI Chatbot",
    "description": "Built an AI-powered chatbot using Python and TensorFlow",
    "techStack": "Python, TensorFlow, Flask, PostgreSQL",
    "role": "Full-stack Developer",
    "startDate": "2023-01",
    "endDate": "2023-06",
    "projectUrl": "https://github.com/johndoe/ai-chatbot",
    "isCurrent": false
  }'
```

### Update Project
```bash
curl -X PUT http://localhost:9080/users/john@example.com/projects/1 \
  -H "Content-Type: application/json" \
  -d '{
    "name": "AI Chatbot v2",
    "description": "Enhanced AI chatbot with new features",
    "techStack": "Python, TensorFlow, FastAPI, PostgreSQL",
    "role": "Lead Developer",
    "startDate": "2023-01",
    "endDate": "2023-08",
    "projectUrl": "https://github.com/johndoe/ai-chatbot-v2",
    "isCurrent": true
  }'
``` 