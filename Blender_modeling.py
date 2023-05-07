import bpy
import math
import socket
import time
import bpy.app.timers

bpy.ops.object.mode_set(mode = 'OBJECT')
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete()

#for update
orig_verts = []

def create_cone(r, h, n, z, l):
    verts = []
    faces = []

    for i in range(n):
        angle = 2 * math.pi * i / n
        x = r * math.cos(angle)
        y = r * math.sin(angle)
        verts.append((x, y, z))
        verts.append((x/1.1, y/1.1, z + h-h/1.1)) 
        verts.append((x/1.3, y/1.3, z + h-h/1.3)) 
        verts.append((x/1.6, y/1.6, z + h-h/1.6))
        verts.append((x/2.1, y/2.1, z + h-h/2.1)) 
        verts.append((x/3, y/3, z + h-h/3))
        orig_verts.append((x, y, z))
        orig_verts.append((x/1.1, y/1.1, z + h-h/1.1)) 
        orig_verts.append((x/1.3, y/1.3, z + h-h/1.3)) 
        orig_verts.append((x/1.6, y/1.6, z + h-h/1.6))
        orig_verts.append((x/2.1, y/2.1, z + h-h/2.1)) 
        orig_verts.append((x/3, y/3, z + h-h/3))
    verts.append((0, 0, z + h))
    orig_verts.append((0, 0, z + h))
    
    # each layer 0~n-1 
    for i in range(0,(l+1)*(n-1), l+1):
        for j in range(0, l):
            faces.append((i+j, i+j+1, i+j+l+2, i+j+l+1))
        faces.append((i+l, i+2*l+1, (l+1)*n))
    
    # each layer n
    for i in range(l):
        faces.append(( (l+1)*(n-1)+i ,(l+1)*(n-1) +i +1, i+1, i))
    faces.append(( (l+1)*n-1, l,  (l+1)*n ))
    

    mesh = bpy.data.meshes.new("Cone")
    mesh.from_pydata(verts, [], faces)
    obj = bpy.data.objects.new("Cone", mesh)
    bpy.context.collection.objects.link(obj)
    cone = bpy.data.objects["Cone"]
    mat = bpy.data.materials.new("Shield Material")
    mat.diffuse_color = (0, 0, 0, 1)
    cone.data.materials.append(mat)

def create_InflatableRing():
    # Ring
    bpy.ops.mesh.primitive_torus_add(location=(0,0,0), major_radius=5, minor_radius=0.4, major_segments=32, minor_segments=8)
    bpy.context.object.name = "Ring"
    ring = bpy.context.active_object
    mat = bpy.data.materials.new("Ring Material")    
    mat.diffuse_color = (1, 1, 1, 1)
    ring.data.materials.append(mat)
    

def create_NoseCone():
    bpy.ops.mesh.primitive_uv_sphere_add(radius=0.5, location=(0, 0, 2.9))
    sphere = bpy.context.active_object
    bpy.context.object.name = "Nose Cone"
    bpy.ops.object.editmode_toggle()
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.mesh.bisect(plane_co=(0,0,3.1), plane_no=(0,0,-1), clear_inner=False, clear_outer=True)
    bpy.ops.object.editmode_toggle()
    mat = bpy.data.materials.new("Nosecone Material")
    mat.diffuse_color = (0, 0, 15, 1)
    sphere.data.materials.append(mat)
    
 
def creat_Heatshield():
    bpy.ops.object.select_all(action='DESELECT')
    cone = bpy.data.objects["Cone"]
    nosecone = bpy.data.objects["Nose Cone"]
    ring = bpy.data.objects["Ring"]
    cone.select_set(True)
    nosecone.select_set(True)
    ring.select_set(True)
    bpy.ops.object.join()
    bpy.context.object.name = "HeatShield"
    #bpy.ops.object.shade_smooth(use_auto_smooth=True)

def creat_sensor(n,l):
    heatshield = bpy.data.objects["HeatShield"]
    vertices = heatshield.data.vertices
    
    num = 0
    step = int( (n/4) * (l + 1) )
    #skip the sphere 193 spots
    initial = 194
    
    for i in range(0, 4*(step-1), step):
        for j in range(5):
            vertex = vertices[initial + i +  j].co 
            x, y, z = vertex
            bpy.ops.mesh.primitive_cube_add(size=0.2, location=(x,y,z+2.9))
            sensor = bpy.context.active_object
            name = "sensor" + str(num)
            bpy.context.object.name = name
            mat = bpy.data.materials.new("sensor Material")
            mat.diffuse_color = (1, 0, 0, 1)
            sensor.data.materials.append(mat)
            num += 1

def update_sensor(n,l):
    heatshield = bpy.data.objects["HeatShield"]
    vertices = heatshield.data.vertices
    
    num = 0
    step = int( (n/4) * (l + 1) )
    #skip the sphere 193 spots
    initial = 194
    
    for i in range(0, 4*(step-1), step):
        for j in range(5):
            vertex = vertices[initial + i +  j].co 
            x, y, z = vertex
            name = "sensor" + str(num)
            cube = bpy.data.objects[name]
            cube.location=(x,y,z+2.9)
            
            num += 1

def creat_cubesat():
    # Create the cube
    bpy.ops.mesh.primitive_cube_add(size=2, enter_editmode=False, location=(0, 0, -3))
    cube = bpy.context.active_object
    cube.name = "Cube"

    # Create the cuboid
    bpy.ops.mesh.primitive_cube_add(size=0.5, enter_editmode=False, location=(0, 0, 0))
    cuboid = bpy.context.active_object
    cuboid.name = "Cuboid"
    bpy.context.active_object.scale = (1, 1, 12)

    # Select both objects
    bpy.ops.object.select_all(action='DESELECT')
    cube.select_set(True)
    cuboid.select_set(True)

    # Join the objects into one
    bpy.ops.object.join()

    # Rename the combined object
    combined_object = bpy.context.active_object
    combined_object.name = "cubesat"
    cubesat = bpy.data.objects["cubesat"]
    mat = bpy.data.materials.new("cubesat Material")
    mat.diffuse_color = (0, 0, 0, 1)
    cubesat.data.materials.append(mat)



def sensor_read(sensor_nr,data_x,data_y,n,l):
    #skip the sphere 193 spots
    initial = 194
    sin30 = 0.5
    sin60 = 0.866
    
    #find the index
    step = int( (n/4) * (l + 1) )
    index = initial + math.floor(sensor_nr/5)*step + (sensor_nr%5) 
    
    #find the obj
    heatshile = bpy.data.objects["HeatShield"]
    vertices = heatshile.data.vertices
        
    #calculate axis location
    new_index = index 
    new_vertex = vertices[new_index].co
    new_x, new_y, new_z = new_vertex 
    decay_factor = 1
    vertices[new_index].co = (new_x, new_y - data_y * decay_factor, new_z )
    
    #neighbor horizontal
    #first calculate the distant of the farthest effected neighbor
    max_distant = int( (n/4 -1) * (l + 1) )
    
    #sensor 1~5    
    #calculate the effect base on right distant
    max_step = int( n/4 -1 ) 
    for i in range(max_step + 1):
        #new index
        new_index = index + i * (l+1) 
        new_vertex = vertices[new_index].co
        new_x, new_y, new_z = new_vertex 
        # old position
        old_x, old_y, old_z = orig_verts[new_index-initial]
        
        #calculate the decay effect on neighbor
        decay_factor = (max_step - i) / max_step
        #calculate angel of the point, for this step we only achieve 90 degree so pi/2 is used
        angel = math.pi /2 * i / max_step
        x_vect = math.cos(angel)
        y_vect = math.sin(angel)
        #calculate final effect(+/-) base on the original point
        if(old_x >= 0):
            eff_x = data_x * decay_factor
        else:
            eff_x = -data_x * decay_factor
        if(old_y >= 0):
            eff_y = data_x * decay_factor
        else:
            eff_y = -data_x * decay_factor  
        vertices[new_index].co = (new_x - eff_x * x_vect * sin30, new_y - (eff_y * sin30 + data_y * decay_factor) * y_vect,new_z - data_x * sin60 * decay_factor )
    #right end
    #left, since index will jump to 3*(n/4)*(l+1), we need last index 
    last_index = index + int( 4 * (n/4) * (l+1) - (l+1) )
    for i in range(max_step):
        #new index
        new_index = last_index - i * (l+1) 
        new_vertex = vertices[new_index].co
        new_x, new_y, new_z = new_vertex
        # old position
        old_x, old_y, old_z = orig_verts[new_index-initial]
        
        #calculate the decay effect on neighbor
        decay_factor = (max_step - i -1) / max_step
        #calculate angel of the point, for this step we only achieve 90 degree so pi/2 is used
        angel = math.pi /2 * (i+1) / max_step
        x_vect = math.cos(angel)
        y_vect = math.sin(angel)
        #calculate final effect(+/-) base on the original point
        if(old_x >= 0):
            eff_x = data_x * decay_factor
        else:
            eff_x = -data_x * decay_factor
        if(old_y >= 0):
            eff_y = data_x * decay_factor
        else:
            eff_y = -data_x * decay_factor   
        vertices[new_index].co = (new_x - eff_x * x_vect * sin30, new_y - (eff_y * sin30 + data_y * decay_factor) * y_vect,new_z - data_x * sin60 * decay_factor )
    #left hand
    
    #update all new position
    heatshile.data.update()

def sensor_read2(sensor_nr,data,n,l):
    #skip the sphere 193 spots
    initial = 194
    #find the index
    step = int( (n/4) * (l + 1) )
    index = initial + math.floor(sensor_nr/5)*step + (sensor_nr%5) 
    
    #find the obj
    heatshile = bpy.data.objects["HeatShield"]
    vertices = heatshile.data.vertices
    
    #neighbor horizontal
    #first calculate the distant of the farthest effected neighbor
    max_distant = int( (n/4 -1) * (l + 1) )
    
    #calculate the effect base on right distant
    max_step = int( n/4 -1 ) 
    for i in range(max_step):
        #new index
        new_index = index + i * (l+1)
        new_vertex = vertices[new_index].co
        new_x, new_y, new_z = new_vertex 
        #calculate the decay effect on neighbor
        decay_factor = (max_step - i) / max_step
        #calculate angel of the point, for this step we only achieve 90 degree so pi/2 is used
        angel = math.pi /2 * i / max_step + math.pi /2 * math.floor(sensor_nr/5)
        x_vect = math.cos(angel)
        y_vect = math.sin(angel)
        #calculate final effect(+/-) base on the original point
        #if(new_x >= 0):
        eff_x = data * decay_factor
        #else:
        #    eff_x = -data * decay_factor
        #if(new_y >= 0):
        eff_y = data * decay_factor
        #else:
        #    eff_y = -data * decay_factor    
        vertices[new_index].co = (new_x - eff_x * x_vect, new_y - eff_y * y_vect,new_z)
    #right end
    for i in range(max_step):
        #new index
        new_index = index - i * (l+1) - (l+1)
        new_vertex = vertices[new_index].co
        new_x, new_y, new_z = new_vertex 
        #calculate the decay effect on neighbor
        decay_factor = (max_step - i) / max_step
        #calculate angel of the point, for this step we only achieve 90 degree so pi/2 is used
        angel = math.pi /2 * math.floor(sensor_nr/5) - math.pi /2 * i / max_step 
        x_vect = math.cos(angel)
        y_vect = math.sin(angel)
        #calculate final effect(+/-) base on the original point
        #if(new_x >= 0):
        eff_x = data * decay_factor
        #else:
        #    eff_x = -data * decay_factor
        #if(new_y >= 0):
        eff_y = data * decay_factor
        #else:
        #    eff_y = -data * decay_factor      
        vertices[new_index].co = (new_x - eff_x * x_vect, new_y - eff_y * y_vect,new_z )
    #left hand
        
    #update all new position
    heatshile.data.update()

def sensor_reset(n,l):
    heatshile = bpy.data.objects["HeatShield"]
    vertices = heatshile.data.vertices
    initial = 193
    for i in range(n):
        for j in range(l+1):
            ind = j * n + i 
            new_index = initial + ind 
            x, y, z = orig_verts[ind]
            vertices[new_index].co = (x,y,z-2.9)

def sensor_read_set1(data1_x,data1_y,data2_x,data2_y,data3_x,data3_y,data4_x,data4_y,data5_x,data5_y):
    sensor_read(0,data1_x,data1_y,building_node,layer)
    sensor_read(1,data2_x,data2_y,building_node,layer)
    sensor_read(2,data3_x,data3_y,building_node,layer)
    sensor_read(3,data4_x,data4_y,building_node,layer)
    sensor_read(4,data5_x,data5_y,building_node,layer)

def sensor_read_set2(d1,d2,d3,d4,d5):
    sensor_read2(5,d1,building_node,layer)
    sensor_read2(6,d2,building_node,layer)
    sensor_read2(7,d3,building_node,layer)
    sensor_read2(8,d4,building_node,layer)
    sensor_read2(9,d5,building_node,layer)

def sensor_read_set3(d1,d2,d3,d4,d5):
    sensor_read2(10,d1,building_node,layer)
    sensor_read2(11,d2,building_node,layer)
    sensor_read2(12,d3,building_node,layer)
    sensor_read2(13,d4,building_node,layer)
    sensor_read2(14,d5,building_node,layer)

def sensor_read_set4(d1,d2,d3,d4,d5):
    sensor_read2(15,d1,building_node,layer)
    sensor_read2(16,d2,building_node,layer)
    sensor_read2(17,d3,building_node,layer)
    sensor_read2(18,d4,building_node,layer)
    sensor_read2(19,d5,building_node,layer)

#Hyper parameter setting
radius = 5
height = 3
building_node = 40
offset_z = 0.35
#layer between top and bottom
layer = 5

creat_cubesat()
create_cone(radius, height, building_node, offset_z, layer)
create_InflatableRing()
create_NoseCone()
creat_Heatshield()
creat_sensor(building_node,layer)

# for debug
#sensor_read_set1(0.1,0.4,0.4,0.4,0.1)
#sensor_read_set2(0.1,0.4,0.6,0.4,0.1)
#sensor_read_set3(0.1,0.4,0.6,0.4,0.1)
#sensor_read_set4(0.1,0.4,0.6,0.4,0.1)
#sensor_reset(building_node,layer)
#update_sensor(building_node,layer)


# ROS
HOST = '192.168.177.1'
PORT = 9999
BUFFER = 1024

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind((HOST,PORT))
sock.listen(1)

print("connected")

pic_nr = 0


interval = 0.35
def my_timer_function():
    
    conn, addr = sock.accept()
    data = conn.recv(BUFFER).decode()
    data_list = list(map(float, data.split()))
    print("Received data:",data_list)
    sensor_reset(building_node,layer)
    
    # limitation : (0.5,0.9,1.1,0.9,0.45)
 
    sensor_read_set1(data_list[4],data_list[9],data_list[3],data_list[8],data_list[2],data_list[7],data_list[1],data_list[6],data_list[0],data_list[5])
    #sensor_read_set2(data_list[5],data_list[6],data_list[7],data_list[8],data_list[9])
    #sensor_read_set3(data_list[10],data_list[11],data_list[12],data_list[13],data_list[14])
    #sensor_read_set4(data_list[15],data_list[16],data_list[17],data_list[18],data_list[19])
    update_sensor(building_node,layer)
    
    # setting of the output path and file name
    #export_path = "/Users/Administrator/Desktop/see/file"+str(pic_nr)+".obj"
    #export_path = "/Users/Administrator/Desktop/see/file"+str(time)+".obj"
    #export_path = "/Users/Administrator/Desktop/see/file.obj"
    # select output object
    #selected_objects = bpy.ops.object.select_all(action='SELECT')

    # set the product as .obj
    #bpy.ops.export_scene.obj(filepath=export_path, use_selection=True)
    #pic_nr = pic_nr + 1
    
    conn.close()
    
    return interval


bpy.app.timers.register(my_timer_function, first_interval=interval)


