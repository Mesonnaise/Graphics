#include<stdexcept>
#include<fstream>
#include<iostream>
#include<format>
#include<sstream>

#include<glm/vec4.hpp>

#include "Mesh.h"


namespace Engine{
  Mesh::Mesh(std::filesystem::path path){
    if(std::filesystem::exists(path)==false)
      throw std::runtime_error("Mesh file not found: "+path.string());
    
    std::ifstream file(path);
    if(!file.is_open())
      throw std::runtime_error("Failed to open mesh file: "+path.string());
    
    auto ParseTokens=[](const std::string &line,const std::string &delimiter=" ")->std::vector<std::string>{
      size_t startPos=0;
      size_t endPos=0;
      std::vector<std::string> tokens;
      while((endPos=line.find(delimiter,startPos))!=std::string::npos){
        auto segment=line.substr(startPos,endPos-startPos);
        tokens.push_back(segment);
        startPos=endPos+delimiter.length();
      }
      tokens.push_back(line.substr(startPos,line.size()-startPos));
      return tokens;
    };

    auto ParseVertex=[](const std::vector<std::string> &tokens)->glm::vec4{
      if(tokens.size()<4)
        throw std::runtime_error("Invalid vertex data");
      float x=std::stof(tokens[1]);
      float y=std::stof(tokens[2]);
      float z=std::stof(tokens[3]);
      float w=1.0;
      if(tokens.size()>=5)
        w=std::stof(tokens[4]);

      return glm::vec4(x,y,z,w);
    };

    auto ParseUV=[](const std::vector<std::string> &tokens)->glm::vec3{
      if(tokens.size()<2)
        throw std::runtime_error("Invalid UV data");
      float u,v,w=0.0f;

      u=std::stof(tokens[1]);
      if(tokens.size()>2){
        v=std::stof(tokens[2]);

        if(tokens.size()>3)
          w=std::stof(tokens[3]);
      }

      return glm::vec3(u,v,w);
    };

    auto ParseFace=[&](const std::vector<std::string> &tokens){
      if(tokens.size()<4)
        throw std::runtime_error("Invalid face data");
      for(int i=1; i<tokens.size();i++){
        auto set=ParseTokens(tokens[i],"/");
        if(set.size()<1)
          throw std::runtime_error("Invalid face vertex data");

        mFaces.push_back(mVertices[std::stoi(set[0])-1]);
      }
    };

    std::stringstream values;
    std::string line;
    while(std::getline(file,line)){
      std::vector<std::string> tokens=ParseTokens(line);
      
      if(tokens.size()>1){
        if(tokens[0]=="o"){
          mName=line.substr(2,line.size()-2);
        } else if(tokens[0]=="v"){
          mVertices.push_back(ParseVertex(tokens));
        } else if(tokens[0]=="vn"){
          mNormals.push_back(ParseVertex(tokens));
        } else if(tokens[0]=="vt"){
          mUVVertices.push_back(ParseUV(tokens));
        } else if(tokens[0]=="vp"){
          //Not implemented
        } else if(tokens[0]=="f"){
          ParseFace(tokens);
        } else if(tokens[0]=="s"){


        } else {
          std::cout<<line<<std::endl;
        }
      }
    }

  }
  Mesh::~Mesh(){
    // Cleanup resources if necessary
  }
}