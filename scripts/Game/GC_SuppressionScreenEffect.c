class GC_SuppressionScreenEffect : SCR_BaseScreenEffect
{
	protected ImageWidget m_fVignette;
	
	//Blurriness
	protected static bool s_bEnableRadialBlur = true;
	protected static float s_fBlurriness = 1;
	protected static float s_fBlurrinessSize = 1;
	
	protected const string m_sRadialBlurMaterialName = "{B011FE0AD21E2447}UI/Materials/ScreenEffects_BlurPP.emat";
	protected const int m_sRadialBlurPriority = 69;
	
	override void DisplayStartDraw(IEntity owner)
	{
		m_fVignette = ImageWidget.Cast(m_wRoot.FindAnyWidget("GC_vignette"));
		
		Print("GC | m_fVignette: " + m_fVignette);
		
		ChimeraCharacter cc = ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());
		
		int cameraId = cc.GetWorld().GetCurrentCameraId();
		cc.GetWorld().SetCameraPostProcessEffect(cameraId, m_sRadialBlurPriority, PostProcessEffectType.RadialBlur, m_sRadialBlurMaterialName);
		
		//m_fVignette.SetEnabled(true);
		//m_fVignette.SetVisible(true);
		//m_fVignette.SetOpacity(1);
		//m_fVignette.SetMaskProgress(0);
		
		//Vignette
		//Blur
		//Color
		//Flinches
	}
}