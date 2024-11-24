import json
import random
import string

# Function to generate random strings
def random_string(length=20):
    return ''.join(random.choices(string.ascii_letters + string.digits, k=length))

# Function to generate a random JSON object
def generate_object(depth=1, max_depth=3):
    if depth > max_depth:  # Stop recursion early
        return random.choice([random_string(), random.randint(0, 1000), True, False, None])
    
    return {
        random_string(10): random.choice([
            random_string(),  # Random string
            random.randint(0, 1000),  # Random number
            generate_object(depth + 1, max_depth),  # Nested object
            [random_string() for _ in range(5)],  # Small array of strings
            [random.randint(0, 1000) for _ in range(5)],  # Small array of numbers
        ])
        for _ in range(5)  # Smaller number of keys per object
    }

# Function to write JSON incrementally
def write_large_json(filename, num_objects=10):
    with open(filename, 'w') as f:
        f.write('{"root": [')  # Start the JSON array
        for i in range(num_objects):
            obj = generate_object()
            f.write(json.dumps(obj))  # Write one object
            if i < num_objects - 1:
                f.write(',')  # Add a comma between objects
        f.write(']}')  # Close the JSON array

# Generate a smaller example file
write_large_json("src\\large_json.json", num_objects=10000)
