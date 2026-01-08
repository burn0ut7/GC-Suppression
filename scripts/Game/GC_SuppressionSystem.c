class GC_SuppressionSystem : GameSystem
{
	[Attribute("10", UIWidgets.Auto, "Max distance range in meters for a bullet to apply suppression")]
	protected float m_fMaxRange;
	
	[Attribute("50", UIWidgets.Auto, "Distance up to which cover is recognized (meters)")]
	protected float m_fCoverTraceLength;
	
	[Attribute("5", UIWidgets.Auto, "For how many seconds suppression can last when fully accumulated")]
	protected float m_fMaxSuppression;
	
	[Attribute("1", UIWidgets.Auto, "Multiplier for the visual intensity of the suppression effect")]
	protected float m_fEffectIntensity;

	//! Projectiles tracked by the system
	protected ref array<GC_ProjectileComponent> m_aProjectiles = {};
	
	//! Player suppression value
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
			.AddPoint(WorldSystemPoint.AfterPhysics);
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
		
		SCR_ChimeraCharacter cc = SCR_ChimeraCharacter.Cast(player);
		vector playerEyePos = cc.EyePosition();
		
		for (int i = m_aProjectiles.Count() - 1; i >= 0; i--)
		{
		
			GC_ProjectileComponent projectile = m_aProjectiles[i];
			
		    vector projPos = projectile.GetOwner().GetOrigin();
		
		    // Check if projectile is moving toward the player
		    float approach = vector.Dot(playerEyePos - projPos, projectile.move.GetVelocity());
			
			if (approach > 0)
				CreateDebugCircle(projPos);
			
		    if (approach <= 0)
		    {
		        // Projectile is no longer approaching → check distance
		        float dist = vector.Distance(projPos, playerEyePos);
				CreateDebugCircle(projPos, Color.GREEN);
				if (dist <= m_fMaxRange)
				{
					// Cover check should move to seperate method so it can be reused for hits around a player
					// "perfect shot" projectile trace towards player head to check whether it would have been in some kind of cover
					TraceParam tp = MakeTraceParam(playerEyePos - projectile.move.GetVelocity().Normalized() * m_fCoverTraceLength, playerEyePos, TraceFlags.ENTS | TraceFlags.WORLD);
					tp.Exclude = player;
					float trace = GetWorld().TraceMove(tp);
					
					AddSuppression(projectile, dist, m_fCoverTraceLength * trace > m_fCoverTraceLength - 0.1);
				}
		
		        // Remove projectile regardless
				Print("GC | Projectile Remove: " + projectile);
		        m_aProjectiles.Remove(i);
		        continue;
		    }
		
		    // Projectile is still approaching → do nothing, keep tracking
		}
	}
	
	void AddSuppression(GC_ProjectileComponent projectile, float distance, bool inCover)
	{
		Print("GC | AddSuppression: " + projectile);
		
		// increase m_fSuppression based on current value, projectile distance and possibly mass
		m_fSuppression = Math.Min(m_fMaxSuppression, m_fSuppression + 0);
		
		//BaseContainer container = projectile.move.GetComponentSource(projectile);
	}
	
	protected int m_iLastUpdate = 0;

	protected void UpdateSuppression()
	{
		// linearily decrease m_fSuppression as time passes
		
		int ms = System.GetTickCount();
		m_fSuppression = Math.Max(0, m_fSuppression - 1000 * (ms - m_iLastUpdate));
		m_iLastUpdate = ms;
		
		// update visual effect intensity based on m_fSuppression and m_fSuppressionIntensity
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
		
			//if (dist <= m_fMaxRange)
			//Get if entity hit nearby, add extra supression
			//Should prob be seperate method
		}
		
		m_aProjectiles.RemoveItem(projectile);
	}

	float GetMaxRange()
	{
		return m_fMaxRange;
	}
	
	//! Debug shapes, remove later
	protected ref array<ref Shape> m_shapes = {};
	protected void CreateDebugCircle(vector position, int color = Color.RED, bool clear = false)
	{
		if(clear)
			m_shapes.Clear();
		
		m_shapes.Insert(Shape.CreateSphere(color, ShapeFlags.DEFAULT, position, 0.2));
	}
}
