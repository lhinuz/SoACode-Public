#include "stdafx.h"
#include "LoadTaskShaders.h"

#include "GameManager.h"

void ProgramGenDelegate::invoke(void* sender, void* userData) {
    std::cout << "Building shader: " << name << std::endl;
    program = new vg::GLProgram(true);
    if (!program->addShader(vs)) {
        errorMessage = "Vertex shader for " + name + " failed to compile.";
        program->dispose();
        delete program;
        return;
    }
    if (!program->addShader(fs)) {
        errorMessage = "Fragment shader for " + name + " failed to compile.";
        program->dispose();
        delete program;
        return;
    }
    if (attr) program->setAttributes(*attr);

    if (!program->link()) {
        errorMessage = name + " failed to link.";
        program->dispose();
        delete program;
        return;
    }
    program->initAttributes();
    program->initUniforms();
}

vg::ShaderSource LoadTaskShaders::createShaderCode(const vg::ShaderType& stage, const IOManager& iom, const cString path, const cString defines /*= nullptr*/) {
    vg::ShaderSource src;
    src.stage = stage;
    if (defines) src.sources.push_back(defines);
    const cString code = iom.readFileToString(path);
    src.sources.push_back(code);
    m_filesToDelete.push_back(code);
    return src;
}

ProgramGenDelegate* LoadTaskShaders::createProgram(nString name, const vg::ShaderSource& vs, const vg::ShaderSource& fs, std::vector<nString>* attr /*= nullptr*/) {
    ProgramGenDelegate& del = m_generators[m_numGenerators];
    m_numGenerators++;

    del.name = name;
    del.vs = vs;
    del.fs = fs;
    del.attr = attr;
    del.rpc.data.f = &del;
    del.rpc.data.userData = nullptr;

    return &del;
}

void LoadTaskShaders::load() {
    vg::GLProgramManager* glProgramManager = GameManager::glProgramManager;

    //***** Attribute Vectors ******
    // So that the same VAO can be used for multiple shaders,
    // it is advantageous to manually set the attributes so that
    // they are the same between several shaders. Otherwise they will
    // be set automatically.

    // Attributes for terrain shaders
    std::vector<nString> terrainAttribs;
    terrainAttribs.push_back("vertexPosition_modelspace");
    terrainAttribs.push_back("vertexUV");
    terrainAttribs.push_back("vertexNormal_modelspace");
    terrainAttribs.push_back("vertexColor");
    terrainAttribs.push_back("vertexSlopeColor");
    terrainAttribs.push_back("texTempRainSpec");

    // Attributes for block shaders
    std::vector<nString> blockAttribs;
    blockAttribs.push_back("position_TextureType");
    blockAttribs.push_back("uvs_animation_blendMode");
    blockAttribs.push_back("textureAtlas_textureIndex");
    blockAttribs.push_back("textureDimensions");
    blockAttribs.push_back("color_waveEffect");
    blockAttribs.push_back("overlayColor");
    blockAttribs.push_back("light_sunlight");
    blockAttribs.push_back("normal");

    // Attributes for texture
    std::vector<nString> dd;
    dd.push_back("vPosition");
    dd.push_back("vUV");

    // Attributes for spherical terrain
    std::vector<nString> sphericalAttribs;
    sphericalAttribs.push_back("vPosition");
    sphericalAttribs.push_back("vColor");

    IOManager iom;

    m_glrpc->invoke(&createProgram("BasicColor",
        createShaderCode(vg::ShaderType::VERTEX_SHADER, iom, "Shaders/BasicShading/BasicColorShading.vert"),
        createShaderCode(vg::ShaderType::FRAGMENT_SHADER, iom, "Shaders/BasicShading/BasicColorShading.frag")
        )->rpc, false);
    m_glrpc->invoke(&createProgram("Texture2D",
        createShaderCode(vg::ShaderType::VERTEX_SHADER, iom, "Shaders/TextureShading/Texture2dShader.vert"),
        createShaderCode(vg::ShaderType::FRAGMENT_SHADER, iom, "Shaders/TextureShading/Texture2dShader.frag")
        )->rpc, false);
    m_glrpc->invoke(&createProgram("Texture",
        createShaderCode(vg::ShaderType::VERTEX_SHADER, iom, "Shaders/TextureShading/TextureShading.vert"),
        createShaderCode(vg::ShaderType::FRAGMENT_SHADER, iom, "Shaders/TextureShading/TextureShading.frag"),
        &dd
        )->rpc, false);

    m_glrpc->invoke(&createProgram("GroundFromAtmosphere",
        createShaderCode(vg::ShaderType::VERTEX_SHADER, iom, "Shaders/TerrainShading/GroundFromAtmosphere.vert"),
        createShaderCode(vg::ShaderType::FRAGMENT_SHADER, iom, "Shaders/TerrainShading/GroundFromAtmosphere.frag"),
        &terrainAttribs
        )->rpc, false);
    m_glrpc->invoke(&createProgram("GroundFromSpace",
        createShaderCode(vg::ShaderType::VERTEX_SHADER, iom, "Shaders/TerrainShading/GroundFromSpace.vert"),
        createShaderCode(vg::ShaderType::FRAGMENT_SHADER, iom, "Shaders/TerrainShading/GroundFromSpace.frag"),
        &terrainAttribs
        )->rpc, false);
    m_glrpc->invoke(&createProgram("Sky",
        createShaderCode(vg::ShaderType::VERTEX_SHADER, iom, "Shaders/AtmosphereShading/Sky.vert"),
        createShaderCode(vg::ShaderType::FRAGMENT_SHADER, iom, "Shaders/AtmosphereShading/Sky.frag")
        )->rpc, false);
    
    vg::ShaderSource vsPostProcess = createShaderCode(vg::ShaderType::VERTEX_SHADER, iom, "Shaders/PostProcessing/PassThrough.vert");
    m_glrpc->invoke(&createProgram("NightVision",
        vsPostProcess,
        createShaderCode(vg::ShaderType::FRAGMENT_SHADER, iom, "Shaders/PostProcessing/NightVision.frag")
        )->rpc, false);
    m_glrpc->invoke(&createProgram("HDR",
        vsPostProcess,
        createShaderCode(vg::ShaderType::FRAGMENT_SHADER, iom, "Shaders/PostProcessing/MotionBlur.frag")
        )->rpc, false);
    m_glrpc->invoke(&createProgram("MotionBlur",
        vsPostProcess,
        createShaderCode(vg::ShaderType::FRAGMENT_SHADER, iom, "Shaders/PostProcessing/MotionBlur.frag", "#define MOTION_BLUR\n")
        )->rpc, false);

    vg::ShaderSource vsBlock = createShaderCode(vg::ShaderType::VERTEX_SHADER, iom, "Shaders/BlockShading/standardShading.vert");
    m_glrpc->invoke(&createProgram("Block",
        vsBlock,
        createShaderCode(vg::ShaderType::FRAGMENT_SHADER, iom, "Shaders/BlockShading/standardShading.frag"),
        &blockAttribs
        )->rpc, false);
    m_glrpc->invoke(&createProgram("Cutout",
        vsBlock,
        createShaderCode(vg::ShaderType::FRAGMENT_SHADER, iom, "Shaders/BlockShading/cutoutShading.frag"),
        &blockAttribs
        )->rpc, false);
    m_glrpc->invoke(&createProgram("Transparency",
        vsBlock,
        createShaderCode(vg::ShaderType::FRAGMENT_SHADER, iom, "Shaders/BlockShading/transparentShading.frag"),
        &blockAttribs
        )->rpc, false);
    m_glrpc->invoke(&createProgram("Sonar",
        vsBlock,
        createShaderCode(vg::ShaderType::FRAGMENT_SHADER, iom, "Shaders/BlockShading/sonarShading.frag"),
        &blockAttribs
        )->rpc, false);

    m_glrpc->invoke(&createProgram("PhysicsBlock",
        createShaderCode(vg::ShaderType::VERTEX_SHADER, iom, "Shaders/PhysicsBlockShading/PhysicsBlockShading.vert"),
        createShaderCode(vg::ShaderType::FRAGMENT_SHADER, iom, "Shaders/BlockShading/standardShading.frag")
        )->rpc, false);
    m_glrpc->invoke(&createProgram("Water",
        createShaderCode(vg::ShaderType::VERTEX_SHADER, iom, "Shaders/WaterShading/WaterShading.vert"),
        createShaderCode(vg::ShaderType::FRAGMENT_SHADER, iom, "Shaders/WaterShading/WaterShading.frag")
        )->rpc, false);
    m_glrpc->invoke(&createProgram("Billboard",
        createShaderCode(vg::ShaderType::VERTEX_SHADER, iom, "Shaders/BillboardShading/BillboardShading.vert"),
        createShaderCode(vg::ShaderType::FRAGMENT_SHADER, iom, "Shaders/BillboardShading/BillboardShading.frag")
        )->rpc, false);
    m_glrpc->invoke(&createProgram("TreeBillboard",
        createShaderCode(vg::ShaderType::VERTEX_SHADER, iom, "Shaders/TreeBillboardShading/TreeBillboardShading.vert"),
        createShaderCode(vg::ShaderType::FRAGMENT_SHADER, iom, "Shaders/TreeBillboardShading/TreeBillboardShading.frag")
        )->rpc, false);
    m_glrpc->invoke(&createProgram("SphericalTerrain",
        createShaderCode(vg::ShaderType::VERTEX_SHADER, iom, "Shaders/SphericalTerrain/SphericalTerrain.vert"),
        createShaderCode(vg::ShaderType::FRAGMENT_SHADER, iom, "Shaders/SphericalTerrain/SphericalTerrain.frag"), 
        &sphericalAttribs
        )->rpc, false);

    // Create all shaders until finished
    for (size_t i = 0; i < m_numGenerators; i++) {
        ProgramGenDelegate& del = m_generators[i];

        // Wait until RPC has finished
        del.rpc.block();

        if (del.program) {
            glProgramManager->addProgram(del.name, del.program);
        } else {
            showMessage(del.errorMessage + " Check command output for more detail. Will attempt to reload.");
            m_glrpc->invoke(&del.rpc, false);
        }
    }

    // Delete loaded files
    for (auto& code : m_filesToDelete) delete[] code;
}
