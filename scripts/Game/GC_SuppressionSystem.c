class GC_SuppressionSystem : GameSystem
{
	[Attribute("10", UIWidgets.Auto, "Max distance range in meters for a bullet to apply suppression")]
	protected float m_maxRange;
	
	protected ref array<GC_ProjectileComponent> m_aProjectiles = {};
	
	protected float m_fSuppression = 0;
	
	static GC_SuppressionSystem GetInstance()
	{
		World world = GetGame().GetWorld();
		if (!world)
			return null;
		return GC_SuppressionSystem.Cast(world.FindSystem(GC_SuppressionSystem));
	}
	
	override void OnInit()
	{
		super.OnInit();
	}

	override static void InitInfo(WorldSystemInfo outInfo)
	{
		super.InitInfo(outInfo);
		outInfo
			.SetAbstract(false)
			.SetUnique(true)
			.SetLocation(WorldSystemLocation.Client)
			.AddPoint(WorldSystemPoint.PostFixedFrame);
	}

	override protected void OnUpdate(ESystemPoint point)
	{
		super.OnUpdate(point);
		
		if (m_aProjectiles.IsEmpty() && m_fSuppression == 0)
			return;
		
		UpdateProjectiles();
		UpdateSuppression();
	}
	
	protected void UpdateProjectiles()
	{
		//Print("GC | UpdateProjectiles: " + m_aProjectiles.Count());
		
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		if (!player)
			return;
		vector playerPos = player.GetOrigin();
		
		for (int i = m_aProjectiles.Count() - 1; i >= 0; i--)
		{
		
			GC_ProjectileComponent projectile = m_aProjectiles[i];
			
		    vector projPos = projectile.GetOwner().GetOrigin();
		
		    // Check if projectile is moving toward the player
		    float approach = vector.Dot(playerPos - projPos, projectile.move.GetVelocity());
		
		    if (approach <= 0)
		    {
		        // Projectile is no longer approaching → check distance
		        float dist = vector.Distance(projPos, playerPos);
				
				// Maybe also LOS check that decreases suppression of the player was behind cover in the direction the bullet came from
		
		        if (dist <= m_maxRange)
					AddSuppression(projectile, dist);
		
		        // Remove projectile regardless
				Print("GC | Projectile Remove: " + projectile);
		        m_aProjectiles.Remove(i);
		        continue;
		    }
		
		    // Projectile is still approaching → do nothing, keep tracking
		}
	}
	
	void AddSuppression(GC_ProjectileComponent projectile, float distance)
	{
		Print("GC | ApplySuppression: " + projectile);
		
		// increase m_fSuppression based on current value, projectile distance and mass
		
		//ShellMoveComponent shellComp = ShellMoveComponent.Cast(projectile.FindComponent(ShellMoveComponent));
		//proj.shell = shellComp.GetComponentSource(projectile);
	}
	
	protected void UpdateSuppression()
	{
		// gradually decrease m_fSuppression as time passes
		// update visual effect intensity based on m_fSuppression
	}
	
	void RegisterProjectile(IEntity effect, BaseMuzzleComponent muzzle, IEntity projectile)
	{
		
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		if (!player)
			return;
		
		GC_ProjectileComponent projComp = GC_ProjectileComponent.Cast(projectile.FindComponent(GC_ProjectileComponent));
		if (!projComp)
			return;
		
		if (!projComp.IsEnabled())
			return;
		
		ProjectileMoveComponent moveComp = ProjectileMoveComponent.Cast(projectile.FindComponent(ProjectileMoveComponent));
		if (!moveComp)
			return;
		
		projComp.effect = effect;
		projComp.muzzle = muzzle;
		projComp.move = moveComp;

		m_aProjectiles.Insert(projComp);
		
		Print("GC | Projectile Registered: " + projectile);
	}
	
	void UnregisterProjectile(GC_ProjectileComponent projectile)
	{
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		if(player)
		{
			vector projPos = projectile.GetOwner().GetOrigin();
			float dist = vector.Distance(projPos, player.GetOrigin());
		
			if (dist <= m_maxRange)
			
		}
		
		m_aProjectiles.RemoveItem(projectile);
	}

	float GetMaxRange()
	{
		return m_maxRange;
	}
}
