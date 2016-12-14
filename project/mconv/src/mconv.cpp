#define TINYOBJLOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "stb_image.h"

#include "../include/mconv.h"

#define V_BIT_CNT 21ULL
#define VT_BIT_CNT 21ULL
#define VN_BIT_CNT 20ULL

using namespace tinyobj;

int32_t index_to_uid ( index_t idx, uint64_t* uid )
{
	uint32_t vertexBits = ((1<<(V_BIT_CNT+1))-1);
	uint32_t texcoordBits = ((1<<(VT_BIT_CNT+1))-1);
	uint32_t normalBits = ((1<<(VN_BIT_CNT+1))-1);

	if ( (idx.vertex_index & vertexBits) != idx.vertex_index
		|| ((idx.texcoord_index & texcoordBits) != idx.texcoord_index && idx.texcoord_index != -1 )
		|| ((idx.normal_index & normalBits) != idx.normal_index && idx.normal_index != -1) )
		return -2;

	*uid = ((uint64_t)idx.vertex_index)
		| ((uint64_t)idx.texcoord_index << V_BIT_CNT)
		| ((uint64_t)idx.normal_index << (V_BIT_CNT + VT_BIT_CNT));
	return 0;
}

// mconv.exe in out
int main ( int argc, char* argv[] )
{
	if ( argc != 3 )
	{
		printf ( "ERROR: Invalid parameters. Correct usage: mconv.exe [in] [out]\n" );
		return -1;
	}

	char* in = argv[1];
	char* out = argv[2];

	uint32_t fileStart = 0;
	for ( int32_t i = strlen ( in ); i >= 0; i-- )
	{
		if ( in[i] == '/' || in[i] == '\\' )
		{
			fileStart = i+1;
			break;
		}
	}
	std::string base ( in, fileStart );

	attrib_t attrib;
	std::vector<shape_t> shapes;
	std::vector<material_t> materials;
	std::string error;

	bool success = LoadObj(&attrib, &shapes, &materials, &error, in, base.c_str ( ));
	if ( !success )
	{
		printf ( "ERROR: Failed to load OBJ file %s\n%s\n", in, error.c_str ( ) );
		return -1;
	}

	FILE* fOut;
	fopen_s ( &fOut, out, "wb" );
	if ( !fOut )
	{
		printf ( "ERROR: Failed to load output file %s\n", out );
		return -1;
	}

	std::map<uint64_t,uint32_t> hashToIdx;
	std::vector<index_t> vertices;
	for ( uint32_t i = 0; i < shapes.size ( ); i++ )
	{
		auto& indices = shapes[i].mesh.indices;
		for ( uint32_t j = 0; j < indices.size ( ); j++ )
		{
			uint64_t uid;
			if ( index_to_uid ( indices[j], &uid ) != 0 )
			{
				printf ( "ERROR: Vertex, texcoord or normal indices out of range\n" );
				return -2;
			}
			auto it = hashToIdx.find ( uid );
			if ( it == hashToIdx.end ( ) )
			{
				hashToIdx[uid] = vertices.size();
				vertices.push_back ( indices[j] );
			}
		}
	}

	struct tex_desc
	{
		uint8_t* pixels;
		int width, height;
	};
	uint32_t texdataSize = 0;
	std::vector<tex_desc> tex;
	std::map<std::string,uint32_t> txIdx;
	for ( uint32_t i = 0; i < materials.size ( ); i++ )
	{
		if ( txIdx.find ( materials[i].diffuse_texname ) == txIdx.end ( ) )
		{
			tex_desc desc = { };
			int dummy;
			desc.pixels = stbi_load (
				(base + materials[i].diffuse_texname).c_str ( ), &desc.width, &desc.height, &dummy, 4
			);
			if ( desc.pixels != NULL )
			{
				txIdx[materials[i].diffuse_texname] = tex.size ( );
				tex.push_back ( desc );
				texdataSize += desc.width * desc.height * 4;
			}
		}
	}

	std::map<uint64_t,uint32_t> hashToObject;
	std::vector<std::pair<uint32_t,int>> objectMaterials;
	for ( uint32_t i = 0; i < shapes.size ( ); i++ )
	{
		auto& indices = shapes[i].mesh.material_ids;
		for ( uint32_t j = 0; j < indices.size ( ); j++ )
		{
			int mat = indices[j];
			if ( mat >= 0 && txIdx.find ( materials[mat].diffuse_texname) == txIdx.end ( ) )
				mat = -1;
			uint64_t hash = ((uint64_t)i << 32) | (uint32_t)mat;
			auto it = hashToObject.find ( hash );
			if ( it == hashToObject.end ( ) )
			{
				hashToObject[hash] = objectMaterials.size ( );
				objectMaterials.push_back ( std::make_pair ( i, mat ) );
			}
		}
	}

	bobj_file_header fhead;
	fhead.magic       = BOBJ_FILE_MAGIC;
	fhead.version     = BOBJ_VERSION;
	fhead.objCount    = objectMaterials.size ( );
	fhead.texCount    = txIdx.size ( );
	fhead.vertexCount = vertices.size ( );

	fhead.objectsStart  = sizeof ( fhead );
	fhead.texturesStart = fhead.objectsStart + fhead.objCount * sizeof ( bobj_object_header );
	fhead.texdataStart  = fhead.texturesStart + fhead.texCount * sizeof ( bobj_texture_header ) + sizeof ( uint32_t );
	fhead.vertexStart   = fhead.texdataStart + texdataSize + sizeof ( uint32_t );
	fhead.indexStart    = fhead.vertexStart + fhead.vertexCount * sizeof ( bobj_vert ) + sizeof ( uint32_t );
	
	fwrite ( &fhead, sizeof ( fhead ), 1, fOut );

	std::vector<bobj_index> indices;

	for ( uint32_t i = 0; i < objectMaterials.size ( ); i++ )
	{
		auto pair = objectMaterials[i];

		bobj_object_header ohead;
		ohead.magic        = BOBJ_OBJECT_MAGIC;
		ohead.indexOffset  = indices.size ( );

		auto& mesh = shapes[pair.first].mesh;
		for ( uint32_t j = 0; j < mesh.material_ids.size ( ); j++ )
		{
			int mat = mesh.material_ids[j];
			if ( mat >= 0 && txIdx.find ( materials[mat].diffuse_texname) == txIdx.end ( ) )
				mat = -1;

			if ( mat == pair.second )
			{
				uint64_t uid[3];
				if ( index_to_uid ( mesh.indices[j*3+0], &uid[0] ) != 0
					|| index_to_uid ( mesh.indices[j*3+1], &uid[1] ) != 0
					|| index_to_uid ( mesh.indices[j*3+2], &uid[2] ) != 0)
				{
					printf ( "ERROR: Vertex, texcoord or normal indices out of range\n" );
					return -3;
				}

				indices.push_back ( hashToIdx[uid[0]] );
				indices.push_back ( hashToIdx[uid[1]] );
				indices.push_back ( hashToIdx[uid[2]] );
			}
		}

		ohead.aabbMin[0] =  FLT_MAX, ohead.aabbMin[1] =  FLT_MAX, ohead.aabbMin[2] =  FLT_MAX;
		ohead.aabbMax[0] = -FLT_MAX, ohead.aabbMax[1] = -FLT_MAX, ohead.aabbMax[2] = -FLT_MAX;

		for ( uint32_t i = ohead.indexOffset; i < indices.size ( ); i++ )
		{
			auto index = indices[i];
			auto vIdx  = vertices[index].vertex_index;
			
			for ( uint32_t j = 0; j < 3; j++ )
			{
				float x = attrib.vertices[vIdx*3+j];
				if ( x < ohead.aabbMin[j] )
					ohead.aabbMin[j] = x;
				if ( x > ohead.aabbMax[j] )
					ohead.aabbMax[j] = x;
			}
		}

		ohead.indexCount   = indices.size ( ) - ohead.indexOffset;
		ohead.textureIndex = pair.second == -1 ? 0xFFFFFFFF : txIdx[materials[pair.second].diffuse_texname];

		fwrite ( &ohead, sizeof ( ohead ), 1, fOut );
	}

	assert ( ftell ( fOut ) == fhead.texturesStart );
	uint32_t texOff = 0;
	for ( uint32_t i = 0; i < tex.size ( ); i++ )
	{
		bobj_texture_header t = { };
		t.magic  = BOBJ_TEXTURE_MAGIC;
		t.width  = tex[i].width;
		t.height = tex[i].height;
		t.offset = texOff;
		texOff  += tex[i].width * tex[i].height * 4;
		fwrite ( &t, sizeof ( t ), 1, fOut );
	}

	uint32_t magic = BOBJ_TEXTURE_DATA_MAGIC;
	fwrite ( &magic, sizeof ( magic ), 1, fOut );
	assert ( ftell ( fOut ) == fhead.texdataStart );
	for ( uint32_t i = 0; i < tex.size ( ); i++ )
	{
		fwrite ( tex[i].pixels, tex[i].width * tex[i].height * 4, 1, fOut );
	}

	magic = BOBJ_VERTEX_DATA_MAGIC;
	fwrite ( &magic, sizeof ( magic ), 1, fOut );
	assert ( ftell ( fOut ) == fhead.vertexStart );
	for ( uint32_t i = 0; i < vertices.size ( ); i++ )
	{
		auto v = vertices[i];
		bobj_vert vert = {};
		
		if ( v.vertex_index != -1 )
		{
			vert.position[0] = attrib.vertices[v.vertex_index*3+0];
			vert.position[1] = attrib.vertices[v.vertex_index*3+1];
			vert.position[2] = attrib.vertices[v.vertex_index*3+2];
		}

		if ( v.texcoord_index != -1 )
		{
			vert.texcoord[0] = attrib.texcoords[v.texcoord_index*2+0];
			vert.texcoord[1] = attrib.texcoords[v.texcoord_index*2+1];
		}

		if ( v.normal_index != -1 )
		{
			vert.normal[0] = attrib.normals[v.normal_index*3+0];
			vert.normal[1] = attrib.normals[v.normal_index*3+1];
			vert.normal[2] = attrib.normals[v.normal_index*3+2];
		}

		fwrite ( &vert, sizeof ( vert ), 1, fOut );
	}

	magic = BOBJ_INDEX_DATA_MAGIC;
	fwrite ( &magic, sizeof ( magic ), 1, fOut );
	assert ( ftell ( fOut ) == fhead.indexStart );
	fwrite ( indices.data(), indices.size ( ) * sizeof ( bobj_index ), 1, fOut );

	// Patch the fileheader to fill in the field we left blank before lalalalalala
	fhead.indexCount = indices.size ( );
	fseek ( fOut, 0, SEEK_SET );
	fwrite ( &fhead, sizeof ( fhead ), 1, fOut );


	fclose ( fOut );

	printf ( "%s ==> %s\n", in, out );
	return 0;
}