
## Create User
Creates a new user with profile details and projects.

```bash
curl -X POST http://localhost:9080/users \
  -H "Content-Type: application/json" \
  -d '{
    "name": "John Doe",
    "email": "john@example.com",
    "bio": "Full-stack developer with 5 years of experience",
    "title": "Senior Software Engineer",
    "location": "New York, USA",
    "skills": "JavaScript, Python, React, Node.js",
    "hourlyRate": "$50",
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

## Update User
Updates an existing user's profile. Note: name and email cannot be updated.

```bash
curl -X PUT http://localhost:9080/users/1 \
  -H "Content-Type: application/json" \
  -d '{
    "name": "John Doe",  
    "email": "john@example.com",  
    "bio": "Updated bio with more experience",
    "title": "Lead Software Engineer",
    "location": "San Francisco, USA",
    "skills": "JavaScript, Python, React, Node.js, AWS",
    "hourlyRate": "$60",
    "availability": "Full-time",
    "experience": "6 years",
    "education": "MS in Computer Science",
    "certifications": "AWS Certified Developer, Google Cloud Professional",
    "githubUrl": "https://github.com/johndoe",
    "linkedinUrl": "https://linkedin.com/in/johndoe",
    "portfolioUrl": "https://johndoe.dev",
    "profilePicture": "https://example.com/new-profile.jpg",
    "projects": [
      {
        "name": "Cloud Migration Project",
        "description": "Led migration of legacy systems to AWS",
        "techStack": "AWS, Docker, Kubernetes",
        "role": "Technical Lead",
        "startDate": "2023-01",
        "endDate": "2023-06",
        "projectUrl": "https://github.com/johndoe/cloud-migration",
        "isCurrent": true
      }
    ]
  }'
```

## Get All Users
Retrieves all users with their complete profiles.

```bash
curl -X GET http://localhost:9080/users
```

## Get Paginated Users
Retrieves users with pagination support.

```bash
# Get first 10 users
curl -X GET "http://localhost:9080/users/list?skip=0&take=10"

# Get next 10 users
curl -X GET "http://localhost:9080/users/list?skip=10&take=10"
```

## Search Users
Performs a fuzzy search across user profiles.

```bash
# Search for Python developers
curl -X GET "http://localhost:9080/users/search?q=python"

# Search for AWS certified professionals
curl -X GET "http://localhost:9080/users/search?q=aws"

# Search by location
curl -X GET "http://localhost:9080/users/search?q=new%20york"
```

## Response Examples

### Successful Response
```json
{
  "system_message": "Current Users",
  "users": [
    {
      "id": 1,
      "name": "John Doe",
      "email": "john@example.com",
      "bio": "Full-stack developer...",
      "title": "Senior Software Engineer",
      "location": "New York, USA",
      "skills": "JavaScript, Python, React, Node.js",
      "hourlyRate": "$50",
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
          "id": 1,
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
    }
  ]
}
```

### Error Response
```json
{
  "error": "Invalid JSON format",
  "details": "parse error at line 1, column 1: syntax error"
}
```

## Notes
1. All responses are in JSON format
2. Error responses include both an error message and details
3. The search endpoint performs case-insensitive fuzzy matching
4. Pagination parameters (skip and take) are optional, defaulting to skip=0 and take=10
5. When updating a user, the name and email fields are preserved and cannot be modified 

### add a prject to a user

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

```bash
curl -X PUT http://localhost:9080/users/john@example.com/projects/2 \
  -H "Content-Type: application/json" \
  -d '{
    "name": "AI Chatbot v2",
    "description": "Enhanced AI chatbot with new features and improved accuracy",
    "techStack": "Python, TensorFlow, FastAPI, PostgreSQL, Redis",
    "role": "Lead Developer",
    "startDate": "2023-01",
    "endDate": "2023-08",
    "projectUrl": "https://github.com/johndoe/ai-chatbot-v2",
    "isCurrent": true
  }'
```