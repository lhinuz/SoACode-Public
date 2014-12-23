#include "stdafx.h"
#include "SphericalTerrainMeshManager.h"

#include "Texture2d.h"

#include "SphericalTerrainPatch.h"

void SphericalTerrainMeshManager::draw(const f64v3& cameraPos, const f32m4& V, const f32m4& VP,
                                       vg::GLProgram* program, vg::GLProgram* waterProgram) {
    
    static float dt = 0.0;
    dt += 0.001;

    if (m_waterMeshes.size()) {
        waterProgram->use();
        waterProgram->enableVertexAttribArrays();

        glUniform1f(waterProgram->getUniform("unDt"), dt);

        glBindTexture(GL_TEXTURE_2D, waterNormalTexture.id);

        for (int i = 0; i < m_waterMeshes.size(); i++) {
            if (m_waterMeshes[i]->m_shouldDelete) {
                // Don't delete here, it will happen in m_meshes
                m_waterMeshes[i] = m_waterMeshes.back();
                m_waterMeshes.pop_back();
                i--;
            } else {
                m_waterMeshes[i]->drawWater(cameraPos, V, VP, waterProgram);
            }
        }

        waterProgram->disableVertexAttribArrays();
        waterProgram->unuse();

    }

    if (m_meshes.size()) {

        program->use();
        program->enableVertexAttribArrays();

        for (int i = 0; i < m_meshes.size(); i++) {
            if (m_meshes[i]->m_shouldDelete) {
                delete m_meshes[i];
                m_meshes[i] = m_meshes.back();
                m_meshes.pop_back();
                i--;
            } else {
                m_meshes[i]->draw(cameraPos, V, VP, program);
            }
        }

        program->disableVertexAttribArrays();
        program->unuse();
    }
}

void SphericalTerrainMeshManager::addMesh(SphericalTerrainMesh* mesh) {
    m_meshes.push_back(mesh);
    if (mesh->m_wvbo) {
        m_waterMeshes.push_back(mesh);
    }
    mesh->m_isRenderable = true;
}