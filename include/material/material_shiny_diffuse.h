#pragma once
/****************************************************************************
 *      This is part of the libYafaRay package
 *
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License as published by the Free Software Foundation; either
 *      version 2.1 of the License, or (at your option) any later version.
 *
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 *
 *      You should have received a copy of the GNU Lesser General Public
 *      License along with this library; if not, write to the Free Software
 *      Foundation,Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef YAFARAY_MATERIAL_SHINY_DIFFUSE_H
#define YAFARAY_MATERIAL_SHINY_DIFFUSE_H

#include "material/material_node.h"

BEGIN_YAFARAY

/*! A general purpose material for basic diffuse and specular reflecting
    surfaces with transparency and translucency support.
    Parameter definitions are as follows:
    Of the incoming Light, the specular reflected part is substracted.
        l' = l*(1.0 - specular_refl)
    Of the remaining light (l') the specular transmitted light is substracted.
        l" = l'*(1.0 - specular_transmit)
    Of the remaining light (l") the diffuse transmitted light (translucency)
    is substracted.
        l"' =  l"*(1.0 - translucency)
    The remaining (l"') light is either reflected diffuse or absorbed.
*/

class ShinyDiffuseMaterial final : public NodeMaterial
{
	public:
		static Material *factory(ParamMap &params, std::list<ParamMap> &params_list, RenderEnvironment &render);

	private:
		ShinyDiffuseMaterial(const Rgb &diffuse_color, const Rgb &mirror_color, float diffuse_strength, float transparency_strength = 0.0, float translucency_strength = 0.0, float mirror_strength = 0.0, float emit_strength = 0.0, float transmit_filter_strength = 1.0, Visibility visibility = Material::Visibility::NormalVisible);
		virtual ~ShinyDiffuseMaterial() override;
		virtual void initBsdf(const RenderState &state, SurfacePoint &sp, BsdfFlags &bsdf_types) const override;
		virtual Rgb eval(const RenderState &state, const SurfacePoint &sp, const Vec3 &wo, const Vec3 &wl, const BsdfFlags &bsdfs, bool force_eval = false) const override;
		virtual Rgb sample(const RenderState &state, const SurfacePoint &sp, const Vec3 &wo, Vec3 &wi, Sample &s, float &w) const override;
		virtual float pdf(const RenderState &state, const SurfacePoint &sp, const Vec3 &wo, const Vec3 &wi, const BsdfFlags &bsdfs) const override;
		virtual bool isTransparent() const override { return m_is_transparent_; }
		virtual Rgb getTransparency(const RenderState &state, const SurfacePoint &sp, const Vec3 &wo) const override;
		virtual Rgb emit(const RenderState &state, const SurfacePoint &sp, const Vec3 &wo) const override; // { return emitCol; }
		virtual void getSpecular(const RenderState &state, const SurfacePoint &sp, const Vec3 &wo, bool &do_reflect, bool &do_refract, Vec3 *const dir, Rgb *const col) const override;
		virtual float getAlpha(const RenderState &state, const SurfacePoint &sp, const Vec3 &wo) const override;
		virtual Rgb getDiffuseColor(const RenderState &state) const override;
		virtual Rgb getGlossyColor(const RenderState &state) const override;
		virtual Rgb getTransColor(const RenderState &state) const override;
		virtual Rgb getMirrorColor(const RenderState &state) const override;
		virtual Rgb getSubSurfaceColor(const RenderState &state) const override;

		struct SdDat
		{
			float component_[4];
			void *node_stack_;
		};

		void config();
		int getComponents(const bool *use_node, NodeStack &stack, float *component) const;
		void getFresnel(const Vec3 &wo, const Vec3 &n, float &kr, float &current_ior_squared) const;

		void initOrenNayar(double sigma);
		float orenNayar(const Vec3 &wi, const Vec3 &wo, const Vec3 &n, bool use_texture_sigma, double texture_sigma) const;

		bool m_is_transparent_ = false;                  //!< Boolean value which is true if you have transparent component
		bool m_is_translucent_ = false;                  //!< Boolean value which is true if you have translucent component
		bool is_mirror_ = false;                       //!< Boolean value which is true if you have specular reflection component
		bool is_diffuse_ = false;                      //!< Boolean value which is true if you have diffuse component

		bool m_has_fresnel_effect_ = false;               //!< Boolean value which is true if you have Fresnel specular effect
		float ior_ = 1.f;                              //!< IOR
		float m_ior_squared_ = 1.f;                     //!< Squared IOR

		bool vi_nodes_[4], vd_nodes_[4];                  //!< describes if the nodes are viewdependant or not (if available)
		ShaderNode *diffuse_shader_ = nullptr;       //!< Shader node for diffuse color
		ShaderNode *bump_shader_ = nullptr;          //!< Shader node for bump
		ShaderNode *transparency_shader_ = nullptr;  //!< Shader node for transparency strength (float)
		ShaderNode *translucency_shader_ = nullptr;  //!< Shader node for translucency strength (float)
		ShaderNode *mirror_shader_ = nullptr;        //!< Shader node for specular reflection strength (float)
		ShaderNode *mirror_color_shader_ = nullptr;   //!< Shader node for specular reflection color
		ShaderNode *sigma_oren_shader_ = nullptr;     //!< Shader node for sigma in Oren Nayar material
		ShaderNode *diffuse_refl_shader_ = nullptr;   //!< Shader node for diffuse reflection strength (float)
		ShaderNode *ior_s_ = nullptr;                 //!< Shader node for IOR value (float)
		ShaderNode *wireframe_shader_ = nullptr;     //!< Shader node for wireframe shading (float)

		Rgb diffuse_color_;              //!< BSDF Diffuse component color
		Rgb emit_color_;                 //!< Emit color
		Rgb mirror_color_;               //!< BSDF Mirror component color
		float mirror_strength_;              //!< BSDF Specular reflection component strength when not textured
		float transparency_strength_;        //!< BSDF Transparency component strength when not textured
		float translucency_strength_;        //!< BSDF Translucency component strength when not textured
		float diffuse_strength_;             //!< BSDF Diffuse component strength when not textured
		float emit_strength_;                //!< Emit strength
		float transmit_filter_strength_;      //!< determines how strong light passing through material gets tinted

		bool use_oren_nayar_ = false;         //!< Use Oren Nayar reflectance (default Lambertian)
		float oren_nayar_a_ = 0.f;           //!< Oren Nayar A coefficient
		float oren_nayar_b_ = 0.f;           //!< Oren Nayar B coefficient

		int n_bsdf_ = 0;

		BsdfFlags c_flags_[4];                   //!< list the BSDF components that are present
		int c_index_[4];                      //!< list the index of the BSDF components (0=specular reflection, 1=specular transparency, 2=translucency, 3=diffuse reflection)
};


END_YAFARAY

#endif // YAFARAY_MATERIAL_SHINY_DIFFUSE_H