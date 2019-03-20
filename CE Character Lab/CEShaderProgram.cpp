#include "CEShaderProgram.hpp"

CE::ShaderProgram::ShaderProgram(const std::string& file_name)
{
    m_program = glCreateProgram();

    m_shaders[Type::VERTEX] = create(load(file_name + ".vs"), Type::VERTEX);
    m_shaders[Type::FRAGMENT] = create(load(file_name + ".fs"), Type::FRAGMENT);
    
    glAttachShader(m_program, m_shaders[Type::VERTEX]);
    glAttachShader(m_program, m_shaders[Type::FRAGMENT]);
    
    glLinkProgram(m_program);
    check(m_program, GL_LINK_STATUS, true, "Error linking shader program");
    
    glValidateProgram(m_program);
    check(m_program, GL_LINK_STATUS, true, "Invalid shader program");
}

CE::ShaderProgram::~ShaderProgram()
{
    for (std::map<Type, GLuint>::iterator shader=m_shaders.begin(); shader != m_shaders.end(); shader++) {
        glDetachShader(m_program, shader->second);
        glDeleteShader(shader->second);
    }
    
    glDeleteProgram(m_program);
}

GLuint CE::ShaderProgram::getProgram() const
{
    return m_program;
}

void CE::ShaderProgram::bind()
{
    glUseProgram(m_program);
}

void CE::ShaderProgram::updateUniforms()
{
    // NOOP
}

std::string CE::ShaderProgram::load(const std::string& fileName)
{
    std::ifstream file;
    file.open((fileName).c_str());
    
    std::string output;
    std::string line;
    
    if(file.is_open())
    {
        while(file.good())
        {
            getline(file, line);
            output.append(line + "\n");
        }
    }
    else
    {
        std::cerr << "Unable to load shader: " << fileName << std::endl;
    }
    
    return output;
}

void CE::ShaderProgram::check(GLuint shader, GLuint flag, bool isProgram, const std::string& errorMessage)
{
    GLint success = 0;
    GLchar error[1024] = { 0 };

    if(isProgram)
        glGetProgramiv(shader, flag, &success);
    else
        glGetShaderiv(shader, flag, &success);

    if(success == GL_FALSE)
    {
        if(isProgram)
            glGetProgramInfoLog(shader, sizeof(error), NULL, error);
        else
            glGetShaderInfoLog(shader, sizeof(error), NULL, error);

        std::cerr << errorMessage << ": '" << error << "'" << std::endl;
    }
}

GLuint CE::ShaderProgram::create(const std::string &content, CE::ShaderProgram::Type type)
{
    GLuint shader = glCreateShader(type);

    if(shader == 0)
        std::cerr << "Error compiling shader type " << type << std::endl;

    const GLchar* p[1];
    p[0] = content.c_str();
    GLint lengths[1];
    lengths[0] = static_cast<int>(content.length());

    glShaderSource(shader, 1, p, lengths);
    glCompileShader(shader);

    check(shader, GL_COMPILE_STATUS, false, "Error compiling shader!");

    return shader;
}
