#include "vaColorScheme.h"

void vaColorScheme::Update() {
    if (!CheckContext())
    {
        m_valid = false;
        return;
    }
    InitProg("transfer_function", "materials_tf.cu", "transfer_function");
    bool valid;
    m_prog = GetProgramByName("transfer_function", valid);

    optix::Buffer tf = vaBasicObject::GetContext()->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, m_colors.size());
    optix::float3 p;
    float *pos = reinterpret_cast<float*> (tf->map());
    for (int i = 0, index = 0; i < static_cast<int>(m_colors.size()); ++i) {
        p = m_colors[i];

        pos[index++] = p.x;
        pos[index++] = p.y;
        pos[index++] = p.z;
    }
    tf->unmap();

    //assign buffer to mapping program
    if (valid)
        m_prog["TFBuffer"]->setBuffer(tf);

    //std::cout << "VALID " << valid << std::endl;
}