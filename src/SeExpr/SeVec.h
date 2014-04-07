#ifndef _vectest2_h_
#define _vectest2_h_
#include <iosfwd>
#include <cstdlib>
#include <cmath>
#include <iostream>

//#############################################################################
// Template Metaprogramming Helpers

//! Static assert error case (false)
template <bool b,class T> struct seexpr_static_assert{};
//! Static assert success case
template <class T> struct seexpr_static_assert<true,T> {typedef T TYPE;};

//! Enable_if success case (can find the type TYPE)
template<bool c,class T=void> struct my_enable_if{typedef T TYPE;};
//! Enable_if failure case (substitution failure is not an error)
template<class T> struct my_enable_if<false,T>{};

//! Static conditional type true case
template<bool c,class T1,class T2> struct static_if{typedef T1 TYPE;};
//! Static conditional type false case
template<class T1,class T2> struct static_if<false,T1,T2>{typedef T2 TYPE;};


//#############################################################################
// Reduction class (helps prevent linear data dependency on reduce unroll)
template<class T,int d> struct Reducer{ 
    static T sum(T* data){T sum=0;for(int k=0;k<d;k++) sum+=data[k];return sum;}
};
template<class T> struct Reducer<T,1>{
    static T sum(T* data){return data[0];}
};
template<class T> struct Reducer<T,2>{
    static T sum(T* data){return data[0]+data[1];}
};
template<class T> struct Reducer<T,3>{
    static T sum(T* data){return data[0]+data[1]+data[2];}
};
template<class T> struct Reducer<T,4>{
    static T sum(T* data){return (data[0]+data[1])+(data[2]+data[3]);}
};

//! SeVec class, generic dimension vector class
//! can also point to data if the template argument ref is true
template<class T,int d,bool ref=false>
class SeVec
{
    // static error types
    struct INVALID_WITH_VECTOR_VALUE{};
    struct INVALID_WITH_VECTOR_REFERENCE{};
    struct INVALID_WITH_DIMENSION{};

    //! internal data (either an explicit arary or a pointer to raw data)
    typename static_if<ref,T*,T[d]>::TYPE x;
public:
    typedef SeVec<T,d,false> T_VEC_VALUE;
    typedef SeVec<T,d,true> T_VEC_REF;

    //! Initialize vector to be reference to plain raw data
    SeVec(T* raw,
        INVALID_WITH_VECTOR_VALUE u=(typename my_enable_if<ref,INVALID_WITH_VECTOR_VALUE>::TYPE()))
        :x(raw)
    {}

    //! Empty constructor (this is invalid for a reference type)
    SeVec(INVALID_WITH_VECTOR_REFERENCE u=(typename my_enable_if<!ref,INVALID_WITH_VECTOR_REFERENCE>::TYPE()))
    {}

    //! Convenience constant vector initialization (valid for any d)
    SeVec(T v0,
        INVALID_WITH_VECTOR_REFERENCE u=(typename my_enable_if<!ref,INVALID_WITH_VECTOR_REFERENCE>::TYPE())){
        for(int k=0;k<d;k++) x[k]=v0;
    }

    //! Convenience 2 vector initialization (only for d==2)
    SeVec(T v1,T v2,
        INVALID_WITH_VECTOR_REFERENCE u=(typename my_enable_if<!ref,INVALID_WITH_VECTOR_REFERENCE>::TYPE())){
        typename seexpr_static_assert<d==2,INVALID_WITH_DIMENSION>::TYPE();
        x[0]=v1;x[1]=v2;}

    //! Convenience 3 vector initialization (only for d==3)
    SeVec(T v1,T v2,T v3,
        INVALID_WITH_VECTOR_REFERENCE u=(typename my_enable_if<!ref,INVALID_WITH_VECTOR_REFERENCE>::TYPE())){
        typename seexpr_static_assert<d==3,INVALID_WITH_DIMENSION>::TYPE();
        x[0]=v1;x[1]=v2;x[2]=v3;}

    //! Convenience 4 vector initialization (only for d==4)
    SeVec(T v1,T v2,T v3,T v4,
        INVALID_WITH_VECTOR_REFERENCE u=(typename my_enable_if<!ref,INVALID_WITH_VECTOR_REFERENCE>::TYPE())){
        typename seexpr_static_assert<d==4,INVALID_WITH_DIMENSION>::TYPE();
        x[0]=v1;x[1]=v2;x[2]=v3;x[3]=v4;}
    // Changed this to default. This is safe! for reference case it makes another reference
    // for value it copies
    //! Copy construct. Only valid if we are not going to be a reference data!
    //SeVec(const SeVec&)
    //{typename static_assert<!ref,INVALID_WITH_VECTOR_REFERENCE>::TYPE();}

    //! Copy construct. Only valid if we are not going to be reference data!
    template<bool refother>
    SeVec(const SeVec<T,d,refother>&  other,
        INVALID_WITH_VECTOR_REFERENCE u=(typename my_enable_if<!ref && refother!=ref,INVALID_WITH_VECTOR_REFERENCE>::TYPE()))
    {*this=other;}

    template<bool refother>
    SeVec& operator=(const SeVec<T,d,refother>& other) 
    {for(int k=0;k<d;k++) x[k]=other[k];return *this;}

    // non-const element access
    T& operator[](const int i){return x[i];}

    // const element access
    const T& operator[](const int i) const {return x[i];}
    
    //! Square of euclidean (2) norm
    T length2() const
    {
        T data[d];
        for(int k=0;k<d;k++) data[k]=x[k]*x[k];
        return Reducer<T,d>::sum(data);
    }
    
    //! Euclidean (2) norm
    T length() const
    {return sqrt(length2());}

    //! Normalize in place and return the 2-norm before normalization
    T normalize()
    {
        T l=length2();
        if(l){
            l=sqrt(l);
            *this/=l;
        }else{
            *this=T_VEC_VALUE((T)0);
            x[0]=1;
        }
        return l;
    }

    //! Return a copy of the vector that is normalized
    SeVec<T,d,false> normalized() const
    {SeVec<T,d,false> other(*this);other.normalize();return other;}

    SeVec& operator/=(const T val){
        T one_over_val=T(1)/val;
        for(int k=0;k<d;k++) x[k]*=one_over_val;
        return *this;
    }

    SeVec& operator*=(const T val){
        for(int k=0;k<d;k++) x[k]*=val;
        return *this;
    }

    template<bool refother>
    SeVec& operator+=(const SeVec<T,d,refother>& other){
        for(int k=0;k<d;k++) x[k]+=other[k];
        return *this;
    }

    template<bool refother>
    SeVec& operator-=(const SeVec<T,d,refother>& other){
        for(int k=0;k<d;k++) x[k]-=other[k];
        return *this;
    }

    template<bool refother>
    SeVec& operator*=(const SeVec<T,d,refother>& other){
        for(int k=0;k<d;k++) x[k]*=other[k];
        return *this;
    }

    template<bool refother>
    SeVec& operator/=(const SeVec<T,d,refother>& other){
        for(int k=0;k<d;k++) x[k]/=other[k];
        return *this;
    }

    T_VEC_VALUE operator-()const
    {T_VEC_VALUE val(*this);for(int k=0;k<d;k++) val[k]=-val[k];return val;}

    template<bool refother>
    bool operator==(const SeVec<T,d,refother>& other) const
    {bool equal=true;for(int k=0;k<d;k++) equal&=(x[k]==other[k]);return equal;}

    template<bool refother>
    bool operator!=(const SeVec<T,d,refother>& other) const
    {return !(*this == other);}

    T_VEC_VALUE operator*(T s) const
    {T_VEC_VALUE val(*this);val*=s;return val;}
    
    T_VEC_VALUE operator/(T s) const
    {T_VEC_VALUE val(*this);val/=s;return val;}

    template<bool refother>
    T_VEC_VALUE operator+(const SeVec<T,d,refother>& other) const
    {T_VEC_VALUE val(*this);val+=other;return val;}

    template<bool refother>
    T_VEC_VALUE operator-(const SeVec<T,d,refother>& other) const
    {T_VEC_VALUE val(*this);val-=other;return val;}

    template<bool refother>
    T_VEC_VALUE operator*(const SeVec<T,d,refother>& other) const
    {T_VEC_VALUE val(*this);val*=other;return val;}

    template<bool refother>
    T_VEC_VALUE operator/(const SeVec<T,d,refother>& other) const
    {T_VEC_VALUE val(*this);val/=other;return val;}

    friend T_VEC_VALUE operator*(T s,const SeVec& v)
    {return v*s;}


    /** Inner product. */
    template<bool refother>
    T dot(const SeVec<T,d,refother>& o) const{
        T vals[d];
        for(int k=0;k<d;k++) vals[d]+=x[k]*o[k];
        T val=0;
        return val;
    }

    /** Cross product. */
    template<bool refother>
    T_VEC_VALUE cross(const SeVec<T,3,refother>& o) const{
        typename seexpr_static_assert<d==3,INVALID_WITH_DIMENSION>::TYPE();
        return T_VEC_VALUE(x[1]*o[2] - x[2]*o[1],
            x[2]*o[0] - x[0]*o[2],
            x[0]*o[1] - x[1]*o[0]); 
    }

    /** Return a vector orthogonal to the current vector. */
    T_VEC_VALUE orthogonal() const{
        typename seexpr_static_assert<d==3,INVALID_WITH_DIMENSION>::TYPE();
        return T_VEC_VALUE(x[1]+x[2],x[2]-x[0],-x[0]-x[1]);
    }

    /**
     * Returns the angle in radians between the current vector and the
     * passed in vector.
     */
    template<bool refother>
    T angle(const SeVec<T,3,refother>& o) const{
        typename seexpr_static_assert<d==3,INVALID_WITH_DIMENSION>::TYPE();
        T l=length()*o.length();
        if(l==0) return 0;
        return acos(dot(o) / l);
    }

    /**
     * Returns the vector rotated by the angle given in radians about
     * the given axis. (Axis must be normalized)
     */
    template<bool refother>
    T_VEC_VALUE rotateBy(const SeVec<T,3,refother>& axis,T angle) const{
        typename seexpr_static_assert<d==3,INVALID_WITH_DIMENSION>::TYPE();
        double c=cos(angle),s=sin(angle);
        return c*(*this)+(1-c)*dot(axis)*axis-s*cross(axis);
    }

    
};

//! Output stream
template<class T,int d,bool r>
std::ostream& operator<<(std::ostream& out,const SeVec<T,d,r>& val)
{
    if(d>0) out<<"("<<val[0];
    for(int k=1;k<d;k++) out<<","<<val[k];
    out<<")";
    return out;
}


#endif