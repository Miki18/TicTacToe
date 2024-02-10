#pragma once

GLuint texture;

GLuint loadTexture(const char* filename) {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    //texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //load a picture
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return 0;
    }

    //Load a BMP
    char header[54];
    file.read(header, 54);
    int width = *(int*)&header[18];
    int height = *(int*)&header[22];

    //Load pixels' data
    int size = 3 * width * height;
    std::vector<char> pixels(size);
    file.read(pixels.data(), size);

    //Create a texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, pixels.data());

    return texture;
}