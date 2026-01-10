class GC_SuppressionScreenEffect : SCR_BaseScreenEffect
{
	[Attribute("1.0", UIWidgets.Auto, "Multiplier applied to vignette mask")]
	protected float m_fVignetteMultiplier;
	
	[Attribute("0.6", UIWidgets.Auto, "Maximum mask for vignette", params: "0 1")]
	protected float m_fVignetteMax;
	
	[Attribute("1.0", UIWidgets.Auto, "Multiplier applied to blur intensivty")]
	protected float m_fBlurMultiplier;
	
	[Attribute("0.2", UIWidgets.Auto, "Maximum blur intensivty", params: "0 0.2")]
	protected float m_fBlurMax;
	
	[Attribute("1.0", UIWidgets.Auto, "Multiplier applied to blur size")]
	protected float m_fBlurSizeMultiplier;
	
	[Attribute("0", UIWidgets.Auto, "Minimum blur size", params: "0 0.5")]
	protected float m_fBlurSizeMin;

	[Attribute("0.25", UIWidgets.Auto, "Minimum saturation", params: "0 1")]
	protected float m_fSaturationMin;
	
	[Attribute("1.0", UIWidgets.Auto, "Multiplier applied to saturation")]
	protected float m_fSaturationMultiplier;
	
	[Attribute("1.1", UIWidgets.Auto, "Minimum contrast", params: "1 2")]
	protected float m_fContrastMax;
	
	[Attribute("1.0", UIWidgets.Auto, "Multiplier applied to blur size")]
	protected float m_fContrastMultiplier;
	
	[Attribute("0.01", UIWidgets.Auto, "Maximum Chromatic Aberration", params: "0 0.05")]
	protected float m_fChromAberMax;
	
	[Attribute("1.0", UIWidgets.Auto, "Multiplier applied to Chromatic Aberration")]
	protected float m_fChromAberMultiplier;

	protected ImageWidget m_wVignette;
	protected ImageWidget m_wFlinch;
	
	//Blur
	protected static bool s_bEnableRadialBlur = false;
	protected static float s_fBlurriness = 0;
	protected static float s_fBlurrinessSize = 0.5;
	
	protected const string m_sRadialBlurMaterialName = "{E0162CBB3FA4AC16}UI/Materials/GC_ScreenEffects_BlurPP.emat";
	protected const int m_sRadialBlurPriority = 17;
	
	//Color
	protected static bool s_bEnableColorEffect = false;
	protected static float s_fContrast = 1;
	protected static float s_fSaturation = 1;
	
	protected const string m_sColorEffectMaterialName = "{C7CAE196FEBD4469}UI/Materials/GC_ScreenEffects_ColorPP.emat";
	protected const int m_sColorEffectPriority = 18;
	
	//Chrom aber
	protected static bool s_bEnableChromAberEffect = false;
	protected static float s_fChromAber = 0;
	
	protected const string m_sChromAberMaterialName = "{07258569ACAB792D}UI/Materials/GC_ScreenEffects_ChromAberPP.emat";
	protected const int m_sChromAberPriority = 19;
	
	
	override void DisplayStartDraw(IEntity owner)
	{
		m_wVignette = ImageWidget.Cast(m_wRoot.FindAnyWidget("GC_vignette"));
		m_wFlinch = ImageWidget.Cast(m_wRoot.FindAnyWidget("GC_flinch"));
		
		ChimeraCharacter cc = ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());
		int CameraId = GetGame().GetWorld().GetCurrentCameraId();

		GetGame().GetWorld().SetCameraPostProcessEffect(CameraId, m_sRadialBlurPriority, PostProcessEffectType.RadialBlur, m_sRadialBlurMaterialName);
		GetGame().GetWorld().SetCameraPostProcessEffect(CameraId, m_sColorEffectPriority, PostProcessEffectType.Colors, m_sColorEffectMaterialName);
		GetGame().GetWorld().SetCameraPostProcessEffect(CameraId, m_sChromAberPriority, PostProcessEffectType.ChromAber, m_sChromAberMaterialName);
	}
	
	void Flinch()
	{
		m_wFlinch.SetMaskProgress(0);
		
		AnimateWidget.StopAllAnimations(m_wFlinch);
		AnimateWidget.AlphaMask(m_wFlinch, 0.9, 8)
			.SetCurve(EAnimationCurve.EASE_IN_CUBIC);
		
		GetGame().GetCallqueue().Remove(ClearFlinch);
		GetGame().GetCallqueue().CallLater(ClearFlinch, 120, false);
	}
	
	void ClearFlinch()
	{
		GetGame().GetCallqueue().Remove(ClearFlinch);
		
		if(!m_wFlinch)
			return;
		
		AnimateWidget.StopAllAnimations(m_wFlinch);
		AnimateWidget.AlphaMask(m_wFlinch, 0, 8)
			.SetCurve(EAnimationCurve.EASE_OUT_CUBIC);	
	}
	
	void UpdateSuppresion()
	{
		float suppressionAmount = GC_SuppressionSystem.GetInstance().GetAmount();

		//Vignette
		float maskProgress = Math.Clamp(suppressionAmount * m_fVignetteMultiplier * m_fVignetteMax, 0, m_fVignetteMax);
		m_wVignette.SetMaskProgress(maskProgress);
		
		// Blur
		s_fBlurriness = Math.Clamp(suppressionAmount * m_fBlurMultiplier * m_fBlurMax, 0, m_fBlurMax);

		float blurSizeT = suppressionAmount * m_fBlurSizeMultiplier;
		blurSizeT = Math.Clamp(blurSizeT, 0, 1);
		s_fBlurrinessSize = Math.Lerp(0.5, m_fBlurSizeMin, blurSizeT);
		
		//Color
		float satT = suppressionAmount * m_fContrastMultiplier;
		satT = Math.Clamp(satT, 0, 1);
		s_fSaturation = Math.Lerp(1.0, m_fSaturationMin, satT);
		
		float conT = suppressionAmount * m_fContrastMultiplier;
		conT = Math.Clamp(conT, 0, 1);
		s_fContrast = Math.Lerp(1.0, m_fContrastMax, conT);

		//Chrom Aber
		s_fChromAber = Math.Clamp(suppressionAmount * m_fChromAberMultiplier * m_fChromAberMax, 0, m_fChromAberMax);
		
		if(suppressionAmount <= 0)
		{
			s_bEnableRadialBlur = false;
			s_bEnableColorEffect = false;
			s_bEnableChromAberEffect = false;
		}
		else
		{
			s_bEnableRadialBlur = true;
			s_bEnableColorEffect = true;
			s_bEnableChromAberEffect = true;
		}
	}
	
	void Disable()
	{
		if(m_wVignette)
			m_wVignette.SetMaskProgress(0);
		
		s_bEnableRadialBlur = false;
		s_bEnableColorEffect = false;
		s_bEnableChromAberEffect = false;
		
		s_fBlurriness = 0;
		s_fBlurrinessSize = 0.5;
		
		s_fContrast = 1;
		s_fSaturation = 1;
		
		s_fChromAber = 0;
		
		ClearFlinch();
	}
}