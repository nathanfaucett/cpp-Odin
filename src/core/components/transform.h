#ifndef _ODIN_TRANSFORM_H_
#define _ODIN_TRANSFORM_H_

namespace Odin {

	class Transform : public Component {

		private:
			int32 m_depth;
			Transform* m_parent;
			Transform* m_root;
			Array<Transform*> m_children;

			bool m_matricesNeedsUpdate;

			inline void m_UpdateDepth(Transform* transform, int32 index);

		protected:
			inline void p_Clear(void);

			inline void p_Init(void);
			inline void p_Start(void);
			inline void p_Update(void);

			inline void p_Sort(void);

		public:
			Vec3f position;
			Quatf rotation;
			Vec3f scale;

			Mat4f matrix;
			Mat4f matrixWorld;
			Mat4f modelView;
			Mat3f normalMatrix;

			inline Transform(void);
			inline ~Transform(void);

			inline void UpdateMatrices(const Mat4f& viewMatrix);

			inline int32 GetDepth(void);

			inline Transform& AddChild(Transform* child);
			inline Transform& RemoveChild(Transform* child);
	};
}

#endif