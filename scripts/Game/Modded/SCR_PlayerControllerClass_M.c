modded class SCR_PlayerController {
	
	override void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		super.OnControlledEntityChanged(from, to);
		
		SCR_ScreenEffectsManager sem = SCR_ScreenEffectsManager.GetScreenEffectsDisplay();
		GC_SuppressionScreenEffect sse = GC_SuppressionScreenEffect.Cast(sem.GetEffect(GC_SuppressionScreenEffect));
		if(sse)
			sse.Disable();
	}
	
}