module;

#include "predef.h"
#define TINYGLTF_IMPLEMENTATION
#include "tiny_gltf.h"
#include "mikktspace.h"

export module ndq:gltf;

import :platform;
import :mesh;
import :image;

#define DEFAULT_BYTES 1024 // 16 X 16 X 4

namespace ndq
{
    struct GLTF
    {
        Mesh MainMesh;
        std::vector<Image> Material;
    };

    int GetNumFaces(const SMikkTSpaceContext* pContext)
    {
        Mesh* mesh = static_cast<Mesh*>(pContext->m_pUserData);
        return static_cast<int>(mesh->Indices.size() / 3);
    }

    int GetNumVerticesOfFace(const SMikkTSpaceContext* pContext, const int iFace)
    {
        return 3;
    }

    void GetPosition(const SMikkTSpaceContext* pContext, float fvPosOut[], const int iFace, const int iVert)
    {
        Mesh* mesh = static_cast<Mesh*>(pContext->m_pUserData);
        DirectX::XMFLOAT3 position = mesh->Positions[mesh->Indices[iFace * 3 + iVert]];
        fvPosOut[0] = position.x;
        fvPosOut[1] = position.y;
        fvPosOut[2] = position.z;
    }

    void GetNormal(const SMikkTSpaceContext* pContext, float fvNormOut[], const int iFace, const int iVert)
    {
        Mesh* mesh = static_cast<Mesh*>(pContext->m_pUserData);
        DirectX::XMFLOAT3 normal = mesh->Normals[mesh->Indices[iFace * 3 + iVert]];
        fvNormOut[0] = normal.x;
        fvNormOut[1] = normal.y;
        fvNormOut[2] = normal.z;
    }

    void GetTexCoords(const SMikkTSpaceContext* pContext, float fvTexcOut[], const int iFace, const int iVert)
    {
        Mesh* mesh = static_cast<Mesh*>(pContext->m_pUserData);
        DirectX::XMFLOAT2 texCoord = mesh->UV0[mesh->Indices[iFace * 3 + iVert]];
        fvTexcOut[0] = texCoord.x;
        fvTexcOut[1] = texCoord.y;
    }

    void SetTSpaceBasic(const SMikkTSpaceContext* pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert)
    {
        Mesh* mesh = static_cast<Mesh*>(pContext->m_pUserData);
        DirectX::XMFLOAT4 tangent = DirectX::XMFLOAT4(fvTangent[0], fvTangent[1], fvTangent[2], fSign);
        int index = mesh->Indices[iFace * 3 + iVert];
        mesh->Tangents[index] = tangent;
    }


    bool CheckGLTF(const GLTF& gltf)
    {
        if (!gltf.MainMesh.Positions.empty() && !gltf.MainMesh.Normals.empty() && !gltf.MainMesh.UV0.empty())
        {
            if (gltf.MainMesh.Positions.size() == gltf.MainMesh.Normals.size() && gltf.MainMesh.Positions.size() == gltf.MainMesh.UV0.size())
            {
                return true;
            }
        }
        return false;
    }

    void GenerateTangents(Mesh& mesh)
    {
        SMikkTSpaceInterface mikkTInterface{};
        mikkTInterface.m_getNumFaces = GetNumFaces;
        mikkTInterface.m_getNumVerticesOfFace = GetNumVerticesOfFace;
        mikkTInterface.m_getPosition = GetPosition;
        mikkTInterface.m_getNormal = GetNormal;
        mikkTInterface.m_getTexCoord = GetTexCoords;
        mikkTInterface.m_setTSpaceBasic = SetTSpaceBasic;

        SMikkTSpaceContext mikkTContext{};
        mikkTContext.m_pInterface = &mikkTInterface;
        mikkTContext.m_pUserData = &mesh;

        mesh.Tangents.resize(mesh.Positions.size());

        genTangSpaceDefault(&mikkTContext);
    }

    void FlipY(Image& image)
    {
        size_type bytesPerPixel = 0;
        switch (image.Format)
        {
        case IMAGE_FORMAT::RGBA_U8:
            bytesPerPixel = 4;
            break;
        }

        if (bytesPerPixel == 0)
        {
            return;
        }

        if (bytesPerPixel <= 4)
        {
            for (size_type i = 0; i < image.RawData.size(); i += bytesPerPixel)
            {
                image.RawData[i + 1] = 255 - image.RawData[i + 1];
            }
        }
    }

    void UpdateGLTF(GLTF& gltf)
    {
        for (auto& position : gltf.MainMesh.Positions)
        {
            position.z = -position.z;
        }
        for (auto& normal : gltf.MainMesh.Normals)
        {
            normal.z = -normal.z;
        }
        for (auto& tangent : gltf.MainMesh.Tangents)
        {
            tangent.z = -tangent.z;
        }
        
        if (gltf.MainMesh.Indices.empty())
        {
            gltf.MainMesh.Indices.resize(gltf.MainMesh.Positions.size());
            std::iota(gltf.MainMesh.Indices.begin(), gltf.MainMesh.Indices.end(), 0);
        }

        for (size_type i = 0; i < gltf.MainMesh.Indices.size(); i += 3)
        {
            std::swap(gltf.MainMesh.Indices[i + 1], gltf.MainMesh.Indices[i + 2]);
        }

        if (gltf.MainMesh.Tangents.empty())
        {
            GenerateTangents(gltf.MainMesh);
        }

        // FilpY
        FlipY(gltf.Material[1]);
    }

    export bool LoadGLTF(const char* path, GLTF& gltf)
    {
        tinygltf::Model model;
        tinygltf::TinyGLTF loader;
        std::string err;
        std::string warn;

        loader.SetImageLoader(tinygltf::LoadImageData, nullptr);
        bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);
        if (ret)
        {
            if (!model.meshes.empty())
            {
                const tinygltf::Mesh& tinyMesh = model.meshes[0];
                for (const auto& primitive : tinyMesh.primitives)
                {
                    for (const auto& attribute : primitive.attributes)
                    {
                        const tinygltf::Accessor& accessor = model.accessors[attribute.second];
                        const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                        const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
                        const auto& byteOffset = accessor.byteOffset + bufferView.byteOffset;
                        const float* bufferFloats = reinterpret_cast<const float*>(&buffer.data[byteOffset]);

                        int count = accessor.count;
                        int numComponents = tinygltf::GetNumComponentsInType(accessor.type);

                        if (attribute.first.compare("POSITION") == 0)
                        {
                            for (int i = 0; i < count; ++i)
                            {
                                gltf.MainMesh.Positions.push_back(DirectX::XMFLOAT3(
                                    bufferFloats[i * numComponents + 0],
                                    bufferFloats[i * numComponents + 1],
                                    bufferFloats[i * numComponents + 2]));
                            }
                        }
                        else if (attribute.first.compare("NORMAL") == 0)
                        {
                            for (int i = 0; i < count; ++i)
                            {
                                gltf.MainMesh.Normals.push_back(DirectX::XMFLOAT3(
                                    bufferFloats[i * numComponents + 0],
                                    bufferFloats[i * numComponents + 1],
                                    bufferFloats[i * numComponents + 2]));
                            }
                        }
                        else if (attribute.first.compare("TEXCOORD_0") == 0)
                        {
                            for (int i = 0; i < count; ++i)
                            {
                                gltf.MainMesh.UV0.push_back(DirectX::XMFLOAT2(
                                    bufferFloats[i * numComponents + 0],
                                    bufferFloats[i * numComponents + 1]));
                            }
                        }
                        else if (attribute.first.compare("TANGENT") == 0)
                        {
                            for (int i = 0; i < count; ++i)
                            {
                                gltf.MainMesh.Tangents.push_back(DirectX::XMFLOAT4(
                                    bufferFloats[i * numComponents + 0],
                                    bufferFloats[i * numComponents + 1],
                                    bufferFloats[i * numComponents + 2],
                                    bufferFloats[i * numComponents + 3]));
                            }
                        }
                    }

                    if (primitive.indices > -1)
                    {
                        const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
                        const tinygltf::BufferView& indexBufferView = model.bufferViews[indexAccessor.bufferView];
                        const tinygltf::Buffer& indexBuffer = model.buffers[indexBufferView.buffer];
                        const uint8* indexBufferData = indexBuffer.data.data() + indexAccessor.byteOffset + indexBufferView.byteOffset;

                        for (size_t i = 0; i < indexAccessor.count; ++i)
                        {
                            uint32_t index;
                            switch (indexAccessor.componentType)
                            {
                            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                                index = reinterpret_cast<const uint8*>(indexBufferData)[i];
                                break;
                            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                                index = reinterpret_cast<const uint16*>(indexBufferData)[i];
                                break;
                            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                                index = reinterpret_cast<const uint32*>(indexBufferData)[i];
                                break;
                            default:
                                // Unsupported index component type
                                continue;
                            }
                            gltf.MainMesh.Indices.push_back(index);
                        }
                    }
                }
            }

            if (!model.materials.empty())
            {
                for (const auto& material : model.materials)
                {
                    if (material.values.find("baseColorTexture") != material.values.end())
                    {
                        Image gltfImage;
                        const auto index = material.pbrMetallicRoughness.baseColorTexture.index;
                        const auto& image = model.images[index];
                        gltfImage.Width = image.width;
                        gltfImage.Height = image.height;
                        if (image.bits == 8 && image.component == 4 && image.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                        {
                            gltfImage.Format = IMAGE_FORMAT::RGBA_U8;
                        }
                        else
                        {
                            gltfImage.Format = IMAGE_FORMAT::UNKNOWN;
                        }
                        gltfImage.RawData = image.image;
                        gltf.Material.emplace_back(gltfImage);
                    }
                    else if(material.values.find("baseColorFactor") != material.values.end())
                    {
                        Image gltfImage;
                        gltfImage.Width = 16;
                        gltfImage.Height = 16;
                        gltfImage.Format = IMAGE_FORMAT::RGBA_U8;

                        const auto& baseColorFactor = material.pbrMetallicRoughness.baseColorFactor;
                        uint8_t R = static_cast<uint8_t>(std::round(baseColorFactor[0] * 255.0));
                        uint8_t G = static_cast<uint8_t>(std::round(baseColorFactor[1] * 255.0));
                        uint8_t B = static_cast<uint8_t>(std::round(baseColorFactor[2] * 255.0));
                        uint8_t A = static_cast<uint8_t>(std::round(baseColorFactor[3] * 255.0));
                        gltfImage.RawData.resize(DEFAULT_BYTES);
                        for (size_type i = 0; i < DEFAULT_BYTES; i += 4)
                        {
                            gltfImage.RawData[i + 0] = R;
                            gltfImage.RawData[i + 1] = G;
                            gltfImage.RawData[i + 2] = B;
                            gltfImage.RawData[i + 3] = A;
                        }

                        gltf.Material.emplace_back(gltfImage);
                    }
                    else
                    {
                        Image gltfImage;
                        gltfImage.Width = 16;
                        gltfImage.Height = 16;
                        gltfImage.Format = IMAGE_FORMAT::RGBA_U8;
                        gltfImage.RawData.resize(DEFAULT_BYTES);
                        for (size_type i = 0; i < DEFAULT_BYTES; i += 4)
                        {
                            gltfImage.RawData[i + 0] = 255;
                            gltfImage.RawData[i + 1] = 0;
                            gltfImage.RawData[i + 2] = 0;
                            gltfImage.RawData[i + 3] = 255;
                        }
                        gltf.Material.emplace_back(gltfImage);
                    }

                    if (material.values.find("metallicRoughnessTexture") != material.values.end())
                    {
                        Image gltfImage;
                        const auto index = material.pbrMetallicRoughness.metallicRoughnessTexture.index;
                        const auto& image = model.images[index];
                        gltfImage.Width = image.width;
                        gltfImage.Height = image.height;
                        if (image.bits == 8 && image.component == 4 && image.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                        {
                            gltfImage.Format = IMAGE_FORMAT::RGBA_U8;
                        }
                        else
                        {
                            gltfImage.Format = IMAGE_FORMAT::UNKNOWN;
                        }
                        gltfImage.RawData = image.image;
                        gltf.Material.emplace_back(gltfImage);
                    }
                    else
                    {
                        uint8 M = 128;
                        uint8 R = 128;
                        if (material.values.find("metallicFactor") != material.values.end())
                        {
                            M = static_cast<uint8>(std::round(material.pbrMetallicRoughness.metallicFactor * 255.0));
                        }
                        if (material.values.find("roughnessFactor") != material.values.end())
                        {
                            R = static_cast<uint8>(std::round(material.pbrMetallicRoughness.roughnessFactor * 255.0));
                        }
                        
                        Image gltfImage;
                        gltfImage.Width = 16;
                        gltfImage.Height = 16;
                        gltfImage.Format = IMAGE_FORMAT::RGBA_U8;
                        gltfImage.RawData.resize(DEFAULT_BYTES);

                        for (size_type i = 0; i < DEFAULT_BYTES; i += 4)
                        {
                            gltfImage.RawData[i + 0] = M;
                            gltfImage.RawData[i + 1] = R;
                            gltfImage.RawData[i + 2] = 0;
                            gltfImage.RawData[i + 3] = 255;
                        }

                        gltf.Material.emplace_back(gltfImage);
                    }

                    if (material.additionalValues.find("normalTexture") != material.additionalValues.end())
                    {
                        const auto index = material.normalTexture.index;
                        const auto& image = model.images[index];

                        Image gltfImage;
                        gltfImage.Width = image.width;
                        gltfImage.Height = image.height;
                        if (image.bits == 8 && image.component == 4 && image.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                        {
                            gltfImage.Format = IMAGE_FORMAT::RGBA_U8;
                        }
                        else
                        {
                            gltfImage.Format = IMAGE_FORMAT::UNKNOWN;
                        }
                        gltfImage.RawData = image.image;
                        gltf.Material.emplace_back(gltfImage);
                    }
                    else
                    {
                        Image gltfImage;
                        gltfImage.Width = 16;
                        gltfImage.Height = 16;
                        gltfImage.Format = IMAGE_FORMAT::RGBA_U8;
                        gltfImage.RawData.resize(DEFAULT_BYTES);
                        for (size_type i = 0; i < DEFAULT_BYTES; i += 4)
                        {
                            gltfImage.RawData[i + 0] = 128;
                            gltfImage.RawData[i + 1] = 128;
                            gltfImage.RawData[i + 2] = 255;
                            gltfImage.RawData[i + 3] = 255;
                        }
                        gltf.Material.emplace_back(gltfImage);
                    }
                }
            }
        }
        // Update
        ret = CheckGLTF(gltf);
        if (ret)
        {
            UpdateGLTF(gltf);
        }

        return ret;
    }
}