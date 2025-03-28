import json
import numpy as np
import trimesh
from scipy.spatial.transform import Rotation as R

def load_obj(file_path):
    """加载 .obj 文件并返回顶点、法线和面数据"""
    mesh = trimesh.load(file_path)
    
    # 如果是地面文件，进行细分
    if 'ground.obj' in file_path:
        # 使用trimesh的细分功能增加顶点密度
        mesh = mesh.subdivide()
        # 可以多次细分以获得更高密度
        mesh = mesh.subdivide()
    
    return mesh.vertices, mesh.vertex_normals, mesh.faces

def apply_transform(vertices, normals, translation, rotation, scale):
    """对顶点和法线应用缩放、旋转和平移变换"""
    # 应用缩放
    vertices = vertices * scale
    # 应用旋转（假设旋转角度为欧拉角，单位是度）
    rotation_matrix = R.from_euler('xyz', rotation, degrees=True).as_matrix()
    vertices = np.dot(vertices, rotation_matrix.T)
    normals = np.dot(normals, rotation_matrix.T)
    # 应用平移
    vertices = vertices + translation
    return vertices, normals

def merge_objects(config):
    """合并所有对象的顶点、法线和面数据，并去除重合顶点"""
    all_vertices = []
    all_normals = []
    all_faces = []
    vertex_map = {}  # 使用字典记录顶点位置到索引的映射
    
    for obj in config["Objects"]:
        # 加载和应用变换
        vertices, normals, faces = load_obj(obj["SourceFile"])
        vertices, normals = apply_transform(
            vertices, normals,
            obj["Translation"], obj["Rotation"], obj["Scale"]
        )
        
        # 创建当前对象的顶点索引映射
        current_vertex_map = np.zeros(len(vertices), dtype=int)
        
        # 批量处理顶点
        rounded_vertices = np.round(vertices, 6)
        for i, v in enumerate(rounded_vertices):
            v_tuple = tuple(v)
            if v_tuple not in vertex_map:
                vertex_map[v_tuple] = len(all_vertices)
                all_vertices.append(vertices[i])
                all_normals.append(normals[i])
            current_vertex_map[i] = vertex_map[v_tuple]
        
        # 批量处理面
        remapped_faces = current_vertex_map[faces]
        all_faces.extend(remapped_faces.tolist())

    return np.array(all_vertices), np.array(all_normals), np.array(all_faces)

def save_obj(file_path, vertices, normals, faces):
    """将合并后的数据保存为 .obj 文件"""
    with open(file_path, 'w') as f:
        # 写入顶点
        for v in vertices:
            f.write(f"v {v[0]} {v[1]} {v[2]}\n")
        # 写入法线
        for n in normals:
            f.write(f"vn {n[0]} {n[1]} {n[2]}\n")
        # 写入面
        for face in faces:
            f.write(f"f {face[0]+1}//{face[0]+1} {face[1]+1}//{face[1]+1} {face[2]+1}//{face[2]+1}\n")

def main():
    Nylon2 = './debug/Nylon2.json'
    ShadowMap = './debug/ShadowMap.json'
    LX = './debug/LX.json'

    # 读取 JSON 配置文件
    with open( LX, 'r') as f:
        config = json.load(f)

    # 合并对象
    vertices, normals, faces = merge_objects(config)

    # 保存合并后的 .obj 文件
    save_obj('merged.obj', vertices, normals, faces)
    print("合并完成，结果已保存为 merged.obj")

if __name__ == "__main__":
    main()
