///
/// BlockTextureLoader.h
/// Seed of Andromeda
///
/// Created by Benjamin Arnold on 16 Jun 2015
/// Copyright 2014 Regrowth Studios
/// All Rights Reserved
///
/// Summary:
/// Loads block textures
///

#pragma once

#ifndef BlockTextureLoader_h__
#define BlockTextureLoader_h__

#include <Vorb/io/IOManager.h>
#include <Vorb/VorbPreDecl.inl>

DECL_VG(class ScopedBitmapResource)

class Block;
class BlockTexturePack;
class ModPathResolver;
class BlockTextureLayer;
struct BlockTexture;

class BlockTextureLoader {
public:
    void init(ModPathResolver* texturePathResolver, BlockTexturePack* texturePack);

    void loadTextureData();

    void loadBlockTextures(Block& block);

    BlockTexture* loadTexture(const nString& filePath);

    void dispose();
private:
    bool loadLayerProperties();
    bool loadTextures();
    bool loadBlockTextureMapping();
    bool loadLayer(BlockTextureLayer& layer);
    bool loadTexFile(const nString& imagePath, BlockTexture* texture);
    bool postProcessLayer(vg::ScopedBitmapResource& bitmap, BlockTextureLayer& layer);


    std::map<nString, BlockTextureLayer> m_layers;

    ModPathResolver* m_texturePathResolver = nullptr;
    BlockTexturePack* m_texturePack = nullptr;
    vio::IOManager m_iom;
};

#endif // BlockTextureLoader_h__
