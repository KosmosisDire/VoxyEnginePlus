#include "prints.as"

class vec3 {
    float x, y, z;

    vec3() {
        x = 0;
        y = 0;
        z = 0;
    }

    vec3(float x, float y, float z) {
        this.x = x;
        this.y = y;
        this.z = z;
    }

    float length() {
        return sqrt(x * x + y * y + z * z);
    }

    vec3 normalize() {
        float len = length();
        return vec3(x / len, y / len, z / len);
    }

    vec3& opAddAssign(const vec3&inout other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return this;
    }

    vec3 opAdd(const vec3& other) {
        return vec3(x + other.x, y + other.y, z + other.z);
    }
}

void main() {
    array<vec3@> vectors = {
        vec3(1, 2, 3),
        vec3(4, 123, 6),
        vec3(7, 8, 9)
    };

    float totalLength = 0;
    vec3 added = vec3(0, 0, 0);
    for (uint i = 0; i < vectors.length(); i++) {
        added = added + vectors[i];
        print(vectors[i].length());
        totalLength += vectors[i].length();
    }

    print("Added Vector: (" + added.x + ", " + added.y + ", " + added.z + ")\n");
    print("Total Length: " + totalLength + "\n");
}
