class Vertex2
{
%TypeHeaderCode
	#include <BALL/VIEW/DATATYPE/vertex2.h>
%End
	public:
	Vertex2() throw();
	Vertex2(const Vertex2& ) throw();
	void clear() throw();
	void set(const Vertex2& ) throw();
	void swap(Vertex2& ) throw();
	void setVertex1(const Vector3& ) throw();
	void setVertex1(const float , const float , const float ) throw();
	Vector3& getVertex1() throw();
	void getVertex1(Vector3& ) const throw();
	void setVertex1Address(const Vector3& ) throw();
	void setDefaultVertex1Address() throw();
	Vector3* getVertex1Address() const throw();
	void setVertex2(const Vector3& ) throw();
	void setVertex2(const float , const float , const float ) throw();
	Vector3& getVertex2() throw();
	void getVertex2(Vector3& ) const throw();
	void setVertex2Address(const Vector3& ) throw();
	void setDefaultVertex2Address() throw();
	Vector3* getVertex2Address() const throw();
	void setVertices(const Vector3& , const Vector3& ) throw();
	void setVertices(const float , const float , const float , const float , const float , const float ) throw();
	void setVertexAddresses(const Vector3& , const Vector3& ) throw();
	void getVertices(Vector3& , Vector3& ) throw();
	void getVertices(float& , float& , float& , float& , float& , float& ) throw();
	void setDefaultVertexAddresses() throw();
	bool isDefaultVertex1Address() const throw();
	bool isDefaultVertex2Address() const throw();
	bool isDefaultVertexAddresses() const throw();
	bool isValid() const throw();
};
