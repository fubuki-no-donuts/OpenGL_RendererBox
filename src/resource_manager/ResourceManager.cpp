#include "ResourceManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

std::map<std::string, Shader>    ResourceManager::m_shaders;
std::map<std::string, Texture2D> ResourceManager::m_textures;

// Load Resources
Shader ResourceManager::LoadShader(const GLchar* vShaderFile, const GLchar* fShaderFile, std::string name)
{
    if (m_shaders.find(name) != m_shaders.end())
    {
        std::cerr << "Warning: Shader '" << name << " already exists!" << std::endl;
        std::cerr << "Return the old Shader!" << std::endl;
        return m_shaders[name];
    }
    m_shaders[name] = loadShaderFromFile(vShaderFile, fShaderFile);
    return m_shaders[name];
}
Texture2D ResourceManager::LoadTexture(const GLchar* file, GLboolean alpha, std::string name)
{
    if (m_textures.find(name) != m_textures.end())
    {
        std::cerr << "Warning: Texture '" << name << " already exists!" << std::endl;
        std::cerr << "Return the old texture!" << std::endl;
        return m_textures[name];
    }
    m_textures[name] = loadTextureFromFile(file, alpha);
    return m_textures[name];
}

Texture2D ResourceManager::LoadCubeMap(std::vector<std::string>& faces, GLboolean alpha, std::string name)
{
    m_textures[name] = loadCubeMapFromFile(faces, alpha);
    return m_textures[name];
}

void ResourceManager::UpdateShader(glm::mat4& projection, glm::mat4& view, glm::vec3& viewPos)
{
    for (auto& shader : m_shaders)
    {
        shader.second.SetMatrix4("projection", projection);
        shader.second.SetMatrix4("view", view);
        shader.second.SetVector3f("viewPos", viewPos);
    }
}

// Get Resources
Shader& ResourceManager::GetShader(std::string name) {
    if (m_shaders.find(name) != m_shaders.end())
        return m_shaders[name]; 
    else
        std::cerr << "Warning: Shader '" << name << "' not found!" << std::endl;
    return m_shaders["default"]; // 返回一个默认的 Shader 对象
}
Texture2D& ResourceManager::GetTexture(std::string name) {
    if(m_textures.find(name) != m_textures.end())
        return m_textures[name];
    else
        std::cerr << "Warning: Texture '" << name << "' not found!" << std::endl;
    return m_textures["default"]; // 返回一个默认的对象
}

void ResourceManager::Clear() {
    // (Properly) delete all shaders	
    for (auto& iter : m_shaders)
        glDeleteProgram(iter.second.m_ID);
    // (Properly) delete all textures
    for (auto& iter : m_textures)
        glDeleteTextures(1, &iter.second.m_ID);
}

Shader ResourceManager::loadShaderFromFile(const GLchar* vShaderFile, const GLchar* fShaderFile) {
    std::string vertexCode;
    std::string fragmentCode;
    try {
        // Open files
        std::ifstream vertexShaderFile(vShaderFile);
        std::ifstream fragmentShaderFile(fShaderFile);
        std::stringstream vShaderStream, fShaderStream;
        // Read file's buffer contents into streams
        vShaderStream << vertexShaderFile.rdbuf();
        fShaderStream << fragmentShaderFile.rdbuf();
        // close file handlers
        vertexShaderFile.close();
        fragmentShaderFile.close();
        // Convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::exception e) {
        std::cout << "ERROR::SHADER: Failed to read shader files" << std::endl;
    }
    const GLchar* vShaderCode = vertexCode.c_str();
    const GLchar* fShaderCode = fragmentCode.c_str();
    // 2. Now create shader object from source code
    Shader shader;
    shader.Compile(vShaderCode, fShaderCode);
    return shader;
}

Texture2D ResourceManager::loadTextureFromFile(const GLchar* file, GLboolean alpha) {
    // Create Texture object
    Texture2D texture;
    // Load image
    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* image = stbi_load(file, &width, &height, &nrComponents, 0);

    GLenum format = GL_RGB;
    if (alpha) {
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
    }
    texture.Internal_Format = format;
    texture.Image_Format = format;

    // Now generate texture
    texture.Generate(width, height, image);
    // And finally free image data
    stbi_image_free(image);
    return texture;
}

Texture2D ResourceManager::loadCubeMapFromFile(std::vector<std::string>& faces, GLboolean alpha) {
    Texture2D cubeMap;
    
    int width, height, format;
    for (unsigned int i = 0; i < faces.size(); i++) {
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &format, 0);
        if (data)
            cubeMap.GenerateCubeMap(width, height, data, i);
        else
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
        stbi_image_free(data);
    }

    return cubeMap;
}
