#ifndef ODIN_MESH_FILTER_H
#define ODIN_MESH_FILTER_H

namespace Odin {

	class MeshFilter : public Component {

		private:
			bool m_visible;
			
		protected:
			inline void p_Init(void);
			inline void p_Start(void);

		public:
			Mesh* mesh;
			Material* material;
			
			inline MeshFilter(void);
			inline MeshFilter(const MeshFilter&) = default;
			inline MeshFilter(MeshFilter&&) = default;
			inline ~MeshFilter(void);

			inline virtual MeshFilter* Clone(void);
			inline virtual MeshFilter& Copy(const MeshFilter& other);
			
			inline void Update(void);
			inline void Clear(void);
			
			inline MeshFilter& operator=(const MeshFilter&)& = default;
			inline MeshFilter& operator=(MeshFilter&&)& = default;
	};
}

#endif