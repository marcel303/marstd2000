//////////////////////////////////////////////////////////////////////
// CMatrix implementation.
//////////////////////////////////////////////////////////////////////

#include <vector>		// std::swap()

#define INDEX3(x, y) ((x)+(y)*3)
#define INDEX4(x, y) ((x)+(y)*4)

inline CMatrix3::CMatrix3() {

}

inline CMatrix3::~CMatrix3() {

}

inline void CMatrix3::make_identity() {

	v[INDEX3(0, 0)] = 1.0;
	v[INDEX3(1, 0)] = 0.0;
	v[INDEX3(2, 0)] = 0.0;

	v[INDEX3(0, 1)] = 0.0;
	v[INDEX3(1, 1)] = 1.0;
	v[INDEX3(2, 1)] = 0.0;

	v[INDEX3(0, 2)] = 0.0;
	v[INDEX3(1, 2)] = 0.0;
	v[INDEX3(2, 2)] = 1.0;

}

inline void CMatrix3::transpose() {

	std::swap(v[INDEX3(1, 0)], v[INDEX3(0, 1)]);
	std::swap(v[INDEX3(2, 0)], v[INDEX3(0, 2)]);
	std::swap(v[INDEX3(2, 1)], v[INDEX3(1, 2)]);

}

inline void CMatrix3::make_rotation(float x, float y, float z) {



}

inline void CMatrix3::make_scaling(float x, float y, float z) {

	v[INDEX3(0, 0)] = x;
	v[INDEX3(1, 1)] = y;
	v[INDEX3(2, 2)] = z;

}

inline void CMatrix3::mul(float x, float y, float z, float* xout, float* yout, float* zout) {
}

inline float& CMatrix3::operator()(int x, int y) {

	return v[INDEX3(x, y)];

}

inline CVector CMatrix3::operator*(CVector& vector) {

	CVector tmp;

	tmp[0] = v[INDEX3(0, 0)] * vector[0] +
		 v[INDEX3(1, 0)] * vector[1] +
		 v[INDEX3(2, 0)] * vector[2];

	tmp[1] = v[INDEX3(0, 1)] * vector[0] +
		 v[INDEX3(1, 1)] * vector[1] +
		 v[INDEX3(2, 1)] * vector[2];

	tmp[2] = v[INDEX3(0, 2)] * vector[0] +
		 v[INDEX3(1, 2)] * vector[1] +
		 v[INDEX3(2, 2)] * vector[2];

	return tmp;

}

inline CMatrix3& CMatrix3::operator*=(CMatrix3& matrix) {

	CMatrix3 tmp;

	for (int i=0; i<3; i++)
		for (int j=0; j<3; j++) {
			tmp.v[INDEX3(j, i)] = 0.0;
			for (int k=0; k<3; k++)
				tmp.v[INDEX3(j, i)] += v[INDEX3(k, i)] * matrix.v[INDEX3(j, k)];
		}

	*this = tmp;

	return *this;

}

inline CMatrix4::CMatrix4() {
}

inline CMatrix4::~CMatrix4() {
}

inline void CMatrix4::make_identity() {

	v[INDEX4(0, 0)] = 1.0;
	v[INDEX4(1, 0)] = 0.0;
	v[INDEX4(2, 0)] = 0.0;
	v[INDEX4(3, 0)] = 0.0;

	v[INDEX4(0, 1)] = 0.0;
	v[INDEX4(1, 1)] = 1.0;
	v[INDEX4(2, 1)] = 0.0;
	v[INDEX4(3, 1)] = 0.0;

	v[INDEX4(0, 2)] = 0.0;
	v[INDEX4(1, 2)] = 0.0;	
	v[INDEX4(2, 2)] = 1.0;
	v[INDEX4(3, 2)] = 0.0;

	v[INDEX4(0, 3)] = 0.0;
	v[INDEX4(1, 3)] = 0.0;
	v[INDEX4(2, 3)] = 0.0;
	v[INDEX4(3, 3)] = 1.0;
	
}

inline void CMatrix4::transpose() {

	std::swap(v[INDEX4(1, 0)], v[INDEX4(0, 1)]);
	std::swap(v[INDEX4(2, 0)], v[INDEX4(0, 2)]);
	std::swap(v[INDEX4(3, 0)], v[INDEX4(0, 3)]);
	std::swap(v[INDEX4(2, 1)], v[INDEX4(1, 2)]);
	std::swap(v[INDEX4(3, 1)], v[INDEX4(1, 3)]);
	std::swap(v[INDEX4(3, 2)], v[INDEX4(2, 3)]);

}

inline void CMatrix4::make_x_rotation(float r) {

	make_identity();
	
	float sinx = sin(r);
	float cosx = cos(r);
	
	v[INDEX4(1, 1)] = cosx;
	v[INDEX4(1, 2)] = -sinx;
 	
	v[INDEX4(2, 1)] = sinx;
	v[INDEX4(2, 2)] = cosx;
	
}

inline void CMatrix4::make_y_rotation(float r) {

	make_identity();
	
	float siny = sin(r);
	float cosy = cos(r);
	
	v[INDEX4(0, 0)] = cosy;
	v[INDEX4(0, 2)] = -siny;
 	
	v[INDEX4(2, 0)] = siny;
	v[INDEX4(2, 2)] = cosy;
	
}

inline void CMatrix4::make_z_rotation(float r) {

	make_identity();
	
	float sinz = sin(r);
	float cosz = cos(r);
	
	v[INDEX4(0, 0)] = cosz;
	v[INDEX4(0, 1)] = sinz;
 	
	v[INDEX4(1, 0)] = -sinz;
	v[INDEX4(1, 1)] = cosz;

}

inline void CMatrix4::make_rotation(float x, float y, float z) {

	make_identity();

	#if 1
	
	// Rotation around x axis.
	
	float sinx = sin(x);
	float cosx = cos(x);
	
	v[INDEX4(1, 1)] = cosx;
	v[INDEX4(1, 2)] = -sinx;
 	
	v[INDEX4(2, 1)] = sinx;
	v[INDEX4(2, 2)] = cosx;
	
	#endif
 	
 	#if 0
 	
	// Rotation around y axis.
	
	float siny = sin(y);
	float cosy = cos(y);
	
	v[INDEX4(0, 0)] = cosy;
	v[INDEX4(0, 2)] = -siny;
 	
	v[INDEX4(2, 0)] = siny;
	v[INDEX4(2, 2)] = cosy;
	
	#endif
	
	#if 0
	
	// Rotation around z axis.
	
	float sinz = sin(z);
	float cosz = cos(z);
	
	v[INDEX4(0, 0)] = cosz;
	v[INDEX4(0, 1)] = sinz;
 	
	v[INDEX4(1, 0)] = -sinz;
	v[INDEX4(1, 1)] = cosz;
	
	#endif	

}

inline void CMatrix4::make_translation(float x, float y, float z) {

	make_identity();
        
	v[INDEX4(3, 0)] = x;
        v[INDEX4(3, 1)] = y;
        v[INDEX4(3, 2)] = z;

}

inline void CMatrix4::make_scaling(float x, float y, float z) {

	make_identity();

        v[INDEX4(0, 0)] = x;
        v[INDEX4(1, 1)] = y;
        v[INDEX4(2, 2)] = z;

}

inline void CMatrix4::mul(float x, float y, float z, float* xout, float* yout, float* zout) {

	*xout = v[INDEX4(0, 0)] * x +
		v[INDEX4(1, 0)] * y +
		v[INDEX4(2, 0)] * z +
		v[INDEX4(3, 0)];

	*yout = v[INDEX4(0, 1)] * x +
		v[INDEX4(1, 1)] * y +
		v[INDEX4(2, 1)] * z +
		v[INDEX4(3, 1)];

	*zout = v[INDEX4(0, 2)] * x +
		v[INDEX4(1, 2)] * y +
		v[INDEX4(2, 2)] * z +
		v[INDEX4(3, 2)];

}

inline void CMatrix4::mul3(float x, float y, float z, float* xout, float* yout, float* zout) {

	*xout = v[INDEX4(0, 0)] * x +
		v[INDEX4(1, 0)] * y +
		v[INDEX4(2, 0)] * z;

	*yout = v[INDEX4(0, 1)] * x +
		v[INDEX4(1, 1)] * y +
		v[INDEX4(2, 1)] * z;

	*zout = v[INDEX4(0, 2)] * x +
		v[INDEX4(1, 2)] * y +
		v[INDEX4(2, 2)] * z;

}

inline float CMatrix4::operator()(int x, int y) {

	return v[INDEX4(x, y)];

}

inline CVector CMatrix4::operator*(CVector& vector) {

	CVector tmp;

	tmp[0] = v[INDEX4(0, 0)] * vector[0] +
		 v[INDEX4(1, 0)] * vector[1] +
		 v[INDEX4(2, 0)] * vector[2] +
		 v[INDEX4(3, 0)];

	tmp[1] = v[INDEX4(0, 1)] * vector[0] +
		 v[INDEX4(1, 1)] * vector[1] +
		 v[INDEX4(2, 1)] * vector[2] +
		 v[INDEX4(3, 1)];

	tmp[2] = v[INDEX4(0, 2)] * vector[0] +
		 v[INDEX4(1, 2)] * vector[1] +
		 v[INDEX4(2, 2)] * vector[2] +
		 v[INDEX4(3, 2)];

	// tmp[3] = 1.0;

	return tmp;

}

inline CMatrix4& CMatrix4::operator*=(CMatrix4& matrix) {

	CMatrix4 tmp;

	for (int i=0; i<4; i++)
		for (int j=0; j<4; j++) {
			tmp.v[INDEX4(j, i)] = v[INDEX4(0, i)] * matrix.v[INDEX4(j, 0)] +
					      v[INDEX4(1, i)] * matrix.v[INDEX4(j, 1)] +
					      v[INDEX4(2, i)] * matrix.v[INDEX4(j, 2)] +
					      v[INDEX4(3, i)] * matrix.v[INDEX4(j, 3)];            			
		}

	*this = tmp;

	return *this;

}

#undef INDEX3
#undef INDEX4

