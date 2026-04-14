enum GC_ESuppressionType
{
	BULLET,
	EXPLOSIVE,
	NONE
}

class GC_SuppressionEffect : BaseProjectileEffect
{
	[Attribute("0", UIWidgets.ComboBox, "Type of suppression to apply", "", ParamEnumArray.FromEnum(GC_ESuppressionType) )]
	GC_ESuppressionType m_eType;
	
	[Attribute("1", UIWidgets.Auto, "Suppression effect multipled from this source", params: "0 inf")]
	protected float m_fMultiplier;

	protected IEntity m_Owner;
	
	override void OnInit(IEntity owner)
	{
		m_Owner = owner;
	}
	
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
			case GC_ESuppressionType.EXPLOSIVE:
				HandleExplosive(source, transform);
				break;
		}
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
		if (shooter == chimera)
			return;
		
		GC_SuppressionSystem suppr = GC_SuppressionSystem.GetInstance();
		if (!suppr)
			return;

		if (shooter && suppr.GetIgnoreFriendlySuppression())
		{
			FactionAffiliationComponent playerFac = FactionAffiliationComponent.Cast(
				chimera.FindComponent(FactionAffiliationComponent));
			FactionAffiliationComponent shooterFac = FactionAffiliationComponent.Cast(
				shooter.FindComponent(FactionAffiliationComponent));

			if (playerFac && shooterFac)
			{
				Faction pf = playerFac.GetAffiliatedFaction();
				Faction sf = shooterFac.GetAffiliatedFaction();
				if (pf && sf && pf == sf)
					return;
			}
		}
		
		float distance = vector.Distance(transform[0], chimera.EyePosition());
		if (distance > suppr.GetMaxRange())
			return;

		suppr.HandleBulletImpact(source, transform, m_fMultiplier, distance, speed);
	}
	
	protected void HandleExplosive(IEntity source, vector transform[3])
	{
		GC_SuppressionSystem suppr = GC_SuppressionSystem.GetInstance();
		if (!suppr)
			return;
		
		suppr.HandleExplosion(source, transform, m_Owner, m_fMultiplier);
	}
}
