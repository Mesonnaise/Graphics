#pragma once
#include<memory>
#include<vector>
#include<filesystem>
#include<string>

#include<glm/vec3.hpp>
#include<glm/vec4.hpp>

namespace Engine{

  class Mesh:public std::shared_ptr<Mesh>{
  private:
    std::string mName;

    std::vector<glm::vec4> mVertices;
    std::vector<glm::vec4> mNormals;
    std::vector<glm::vec3> mUVVertices;

    std::vector<glm::vec3> mFaces;
  protected:
    Mesh(std::filesystem::path path);
  public:
    static inline std::shared_ptr<Mesh> Create(std::filesystem::path path){
      auto p=new Mesh(path);
      return std::shared_ptr<Mesh>(p);
    }

    ~Mesh();
    constexpr std::string GetName()const{
      return mName;
    }

    constexpr const std::vector<glm::vec4> &GetVertices()const{
      return mVertices;
    }
    constexpr const std::vector<glm::vec3> &GetFaces()const{
      return mFaces;
    }
  };

  using MeshPtr=std::shared_ptr<Mesh>;
}

