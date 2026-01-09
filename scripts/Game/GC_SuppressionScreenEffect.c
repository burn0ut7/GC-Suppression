class GC_SuppressionScreenEffect : SCR_BaseScreenEffect
{
	[Attribute("1.0", UIWidgets.Auto, "Multiplier applied to vignette mask")]
	protected float m_fVignetteMultiplier;
	
	[Attribute("0.7", UIWidgets.Auto, "Maxium mask for vignette", params: "0 1")]
	protected float m_fVignetteMax;
	
	protected ImageWidget m_fVignette;
	
	//Blurriness
	protected static bool s_bEnableRadialBlur;
	protected static float s_fBlurriness;
	protected static float s_fBlurrinessSize;
	
	protected const string m_sRadialBlurMaterialName = "{E0162CBB3FA4AC16}UI/Materials/GC_ScreenEffects_BlurPP.emat";
	protected const int m_sRadialBlurPriority = 18;
	
	override void DisplayStartDraw(IEntity owner)
	{
		m_fVignette = ImageWidget.Cast(m_wRoot.FindAnyWidget("GC_vignette"));
		
		Print("GC | m_fVignette: " + m_fVignette);
		
		ChimeraCharacter cc = ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());
		int CameraId = GetGame().GetWorld().GetCurrentCameraId();

		GetGame().GetWorld().SetCameraPostProcessEffect(CameraId, m_sRadialBlurPriority, PostProcessEffectType.RadialBlur, m_sRadialBlurMaterialName);
		
		//m_fVignette.SetEnabled(true);
		//m_fVignette.SetVisible(true);
		//m_fVignette.SetOpacity(1);
		//m_fVignette.SetMaskProgress(0);
		s_bEnableRadialBlur = false;
		s_fBlurriness = 0;
		s_fBlurrinessSize = 0;
	}

	//Vignette
	//Blur
	//Color
	//Flinches

	void UpdateSuppresion()
	{
		float suppressionAmount = GC_SuppressionSystem.GetInstance().GetAmount();
		
		float maskProgress = Math.Clamp(suppressionAmount * m_fVignetteMultiplier * m_fVignetteMax, 0, m_fVignetteMax);
		m_fVignette.SetMaskProgress(maskProgress);
	}
}