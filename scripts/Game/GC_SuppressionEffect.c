enum GC_ESuppressionType
{
	BULLET,
	COLLISION,
	TIME,
	EXPLOSIVE,
	NONE
}

class GC_SuppressionEffect : BaseProjectileEffect
{
	[Attribute("0", UIWidgets.ComboBox, "Type of suppression to apply", "", ParamEnumArray.FromEnum(GC_ESuppressionType) )]
	GC_ESuppressionType m_eType;
	
	[Attribute("Test", UIWidgets.Auto )]
	protected string m_sName;
	
	override void OnEffect(IEntity pHitEntity, inout vector outMat[3], IEntity damageSource, notnull Instigator instigator, string colliderName, float speed)
	{
		HandleEffect(damageSource, outMat, instigator, speed);
	}
	
	void HandleEffect(IEntity source, vector transform[3], Instigator instigator, float speed)
	{
		if (!source)
			return;

		switch(m_eType)
		{
			case GC_ESuppressionType.BULLET:
				HandleBullet(source, transform, instigator, speed);
				break;
			case GC_ESuppressionType.COLLISION:
				HandleCollsion(source, transform, instigator, speed);
				break;
		}
		
		/*
		ProjectileMoveComponent move = ProjectileMoveComponent.Cast(source.FindComponent(ProjectileMoveComponent));
		if(!move)
			return;
		
		BaseContainer container = move.GetComponentSource(source);
		int count = container.GetNumVars();
		for(int i = 0; i < count; i++)
		{
			PrintFormat("GC | %1 VAR:%2", i, container.GetVarName(i));
		}
		
		
		BaseContainer effect;
		BaseContainerList cotainerList = container.GetObjectArray("ProjectileEffects");
		for (int i = 0, num = cotainerList.Count(); i < num; i++)
		{
			BaseContainer cont = cotainerList.Get(i);
			PrintFormat("GC | VAR: %1", cont.GetClassName());
			if (cont.GetClassName() == "ExplosionEffect")
				effect = cont;
		}
		
		string name;
		effect.Get("EffectPrefab", name);
		PrintFormat("GC | EffectPrefab: %1", name);
		*/
	}
	
	protected void HandleBullet(IEntity source, vector transform[3], Instigator instigator, float speed)
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return;
		
		SCR_ChimeraCharacter chimera = SCR_ChimeraCharacter.Cast(pc.GetControlledEntity());
		if (!chimera)
			return;
		
		IEntity shooter = instigator.GetInstigatorEntity();
		if(shooter == chimera)
			return;
		
		GC_SuppressionSystem suppr = GC_SuppressionSystem.GetInstance();
		if (!suppr)
			return;
		
		float distance = vector.Distance(transform[0], chimera.EyePosition());
		if (distance > suppr.GetMaxRange())
			return;

		suppr.HandleBulletImpact(source, transform, distance, speed);
	}
	
	protected void HandleCollsion(IEntity source, vector transform[3], Instigator instigator, float speed)
	{
		CollisionTriggerComponent collsion = CollisionTriggerComponent.Cast(source.FindComponent(CollisionTriggerComponent));
		if(!collsion)
			return;
		
		/*
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return;
		
		SCR_ChimeraCharacter chimera = SCR_ChimeraCharacter.Cast(pc.GetControlledEntity());
		if (!chimera)
			return;
		
		IEntity shooter = instigator.GetInstigatorEntity();
		if(shooter == chimera)
			return;
		
		GC_SuppressionSystem suppr = GC_SuppressionSystem.GetInstance();
		if (!suppr)
			return;
		
		float distance = vector.Distance(transform[0], chimera.EyePosition());
		if (distance > suppr.GetMaxRange())
			return;
		*/
		
		//suppr.HandleBulletImpact(source, transform[1], distance, speed);
	}
}
