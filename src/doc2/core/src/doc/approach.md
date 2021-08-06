Approach overview
============

A scalar field is a mapping \f[ f: x \rightarrow \Re, x \in \Re^n \f], which associates any point in space with a scalar value.
 The conventional Volume Rendering scheme suggests that volume boundary (or scalar field domain) is defined as a bounding 
 box and the ray/volume intersection is computed to define the ray's start and end points to solve the Volume Rendering equation with the ray-marching procedure. 
 In the general case, the scalar field can be defined on an arbitrary domain and the object boundary can have a more complex representation and can change dynamically in time.

Let us consider the case of the molecular structure, that is a result of quantum simulation. 
The molecule is usually defined with electron density field and electrostatic potential fields.
 The entire structure can be treated as heterogeneous object, where first field defines the molecule interaction boundary \f$(F'(X)\f$ , and the second field represents the physical property, 
 namely the charge distribution \f$ S'_1(X)\f$. We take advantage of the  HyperVolume (HV) model to define the initial structure and the visual-auditory pipeline, where initial data is represented with HV model: 
\f$ o(t)=(G(t),A_1(t)): (F'(X|t),S'_1(X|t))\f$

,where t - is time parameter as in general case the researcher has to deal with dynamic molecules. 

Via application of functional mapping procedures, that define optic and auditory transfer function, we receive a HV representation:
\f$ m(t)=(G(t),A_o(t),A_s(t)): (F(X|t),S_o(X|t),S_s(X|t)) \f$

, that represents an abstract heterogeneous object with visual and auditory properties. 




Rendering optical model and geometry
------------

Optix engine, optical materials
 
Auditory rendering
------------

Optix engine, auditory materials


