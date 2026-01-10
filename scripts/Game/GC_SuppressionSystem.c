class GC_SuppressionSystem : GameSystem
{
	[Attribute("10", UIWidgets.Auto, "Max distance range in meters for a bullet to apply suppression passing by")]
	protected float m_fMaxRange;
	
	[Attribute("10", UIWidgets.Auto, "Min distance range in meters for a bullet to apply suppression from origin")]
	protected float m_fMinRange;
	
	[Attribute("1", UIWidgets.Auto, "Distance range in meters for a bullet to apply flinch")]
	protected float m_fFlinchRange;
	
	[Attribute("0.025", UIWidgets.Auto, "Random percent angle for aiming when flinched")]
	protected float m_fFlinchRandom;

	[Attribute("5.0", UIWidgets.Auto, "Distance up to which cover is recognized (meters)")]
	protected float m_fCoverTraceLength;
	
	[Attribute("5.0", UIWidgets.Auto, "Seconds of no new suppression before recovery starts")]
	protected float m_fRecoveryDelay;
	
	[Attribute("0.2", UIWidgets.Auto, "Suppression recovery speed per second")]
	protected float m_fRecoveryRate;

	[Attribute("2.0", UIWidgets.Auto, "Multiplier applied to projectile mass for suppression")]
	protected float m_fMassMultiplier;
	
	[Attribute("0.001", UIWidgets.Auto, "Multiplier applied to projectile speed for suppression")]
	protected float m_fSpeedMultiplier;
	
	[Attribute("0.035", UIWidgets.Auto, "Global suppression multiplier per bullet")]
	protected float m_fBaseSuppressionMultiplier;
	
	[Attribute("0.5", UIWidgets.Auto, "Suppression multiplier when target is in cover (0.5 = -50%)")]
	protected float m_fCoverMultiplier;

	//! Projectiles tracked by the system
	protected ref array<GC_ProjectileComponent> m_aProjectiles = {};
	
	//! Player suppression value 0-1
	protected float m_fSuppression = 0;
	
	protected int m_iLastUpdateMs = 0;
	protected int m_iLastSuppressionMs = 0;
	
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
		
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!pc)
			return;

		pc.m_OnControlledEntityChanged.Insert(OnControlledEntityChanged);
	}
	
	void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		m_fSuppression = 0;
		
		SCR_ScreenEffectsManager sem = SCR_ScreenEffectsManager.GetScreenEffectsDisplay();
		GC_SuppressionScreenEffect sse = GC_SuppressionScreenEffect.Cast(sem.GetEffect(GC_SuppressionScreenEffect));
		if (sse)
			sse.Disable();
	}

	override static void InitInfo(WorldSystemInfo outInfo)
	{
		super.InitInfo(outInfo);
		outInfo
			.SetAbstract(false)
			.SetUnique(true)
			.SetLocation(WorldSystemLocation.Client)
			.AddPoint(WorldSystemPoint.FixedFrame);
	}

	override protected void OnUpdate(WorldSystemPoint point)
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
			
		    vector projPos = projectile.entity.GetOrigin();
		
		    // Check if projectile is moving toward the player
		    float approach = vector.Dot(playerEyePos - projPos, projectile.move.GetVelocity());
			
		    if (approach <= 0)
		    {
				// Projectile is no longer approaching → check distance
				vector flybyPos = GetFlybyPoint(projPos, projectile.position, playerEyePos);
				float dist = vector.Distance(flybyPos, playerEyePos);
				
				if (dist <= m_fMaxRange)
				{
					float multi = 1;
					if(IsInCover(projectile))
						multi -= m_fCoverMultiplier;
					
					AddSuppression(projectile, dist, multi);
					CreateDebugCircle(flybyPos, Color.GREEN);
				}
				else
					CreateDebugCircle(flybyPos, Color.PINK);
				
				if (dist <= m_fFlinchRange)
					Flinch();

		        m_aProjectiles.Remove(i);
		        continue;
		    }
			else
				CreateDebugCircle(projPos);
		
			projectile.position = projPos;
		}
	}
	
	protected void AddSuppression(GC_ProjectileComponent projectile, float distance, float multiplier = 1)
	{
		if (!projectile)
			return;

		float mass = GetMass(projectile);
		if (!mass)
			return;
		
		// fucky because ondelete gets rid of move component instance
		float speed = projectile.move.GetVelocity().Length();

		float distanceTerm = 0.0;
		if (distance <= 0.0)
			distanceTerm = 1.0;
		else if (distance < m_fMaxRange)
			distanceTerm = 1.0 - (distance / m_fMaxRange);

		float addSuppression = 0.0;
		addSuppression += (mass * m_fMassMultiplier);
		addSuppression += (speed * m_fSpeedMultiplier);
		addSuppression *= distanceTerm;
	
		// Global scale
		addSuppression = addSuppression * m_fBaseSuppressionMultiplier * multiplier;
		
		m_fSuppression = Math.Clamp(m_fSuppression + addSuppression, 0, 1);
		
		m_iLastSuppressionMs = System.GetTickCount();
		
		UpdateEffect();
		
		PrintFormat("GC | AddSuppression mass=%1 speed=%2 dist=%3 multiplier=%4 add=%5 total=%6 porj=%7",
			mass, speed, distance, multiplier, addSuppression, m_fSuppression, projectile.entity);
	}
	
	//This is also fucky where again movecomp instance is gone when under going delete
	protected float GetMass(GC_ProjectileComponent projectile)
	{
		EntityPrefabData pefab = projectile.entity.GetPrefabData();

		BaseContainer pefabContainer = pefab.GetPrefab();

		BaseContainerList components = pefabContainer.GetObjectArray("components");

		BaseContainer container;
		for (int c = components.Count() - 1; c >= 0; c--)
		{
			container = components.Get(c);
			typename type = container.GetClassName().ToType();
			
			if (type && type.IsInherited(ProjectileMoveComponent))
				break;
			
			container = null;
		}
		
		float mass;
		container.Get("Mass", mass);
		
		return mass;
	}
	
	protected void UpdateSuppression()
	{
		int now = System.GetTickCount();
	
		float deltaSec = (now - m_iLastUpdateMs) / 1000.0;
		m_iLastUpdateMs = now;
	
		if (m_fSuppression <= 0)
			return;
	
		float sinceSuppSec = (now - m_iLastSuppressionMs) / 1000.0;
	
		if (sinceSuppSec < m_fRecoveryDelay)
			return;
	
		float decay = m_fRecoveryRate * deltaSec;

		m_fSuppression = Math.Clamp(m_fSuppression - decay, 0, 1);
		
		UpdateEffect();
	}
	
	protected void UpdateEffect()
	{
		SCR_ScreenEffectsManager sem = SCR_ScreenEffectsManager.GetScreenEffectsDisplay();
		GC_SuppressionScreenEffect sse = GC_SuppressionScreenEffect.Cast(sem.GetEffect(GC_SuppressionScreenEffect));
		if (sse)
			sse.UpdateSuppresion();
	}
	
	protected void Flinch()
	{
		SCR_ScreenEffectsManager sem = SCR_ScreenEffectsManager.GetScreenEffectsDisplay();
		GC_SuppressionScreenEffect sse = GC_SuppressionScreenEffect.Cast(sem.GetEffect(GC_SuppressionScreenEffect));
		if (sse)
			sse.Flinch();
		
		if (m_fFlinchRandom == 0)
			return;
		
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		CharacterControllerComponent cc = CharacterControllerComponent.Cast(player.FindComponent(CharacterControllerComponent));

		if (!cc.GetInputContext().IsWeaponADS())
			return;
		
		vector angles = cc.GetInputContext().GetAimingAngles();

		float randomX = Math.RandomFloatInclusive(1 - m_fFlinchRandom, 1 + m_fFlinchRandom);
		float randomY = Math.RandomFloatInclusive(1 - m_fFlinchRandom, 1 + m_fFlinchRandom);
		float randomZ = Math.RandomFloatInclusive(1 - m_fFlinchRandom, 1 + m_fFlinchRandom);
		
		angles[0] = angles[0] * randomX;
		angles[1] = angles[1] * randomY;
		angles[2] = angles[2] * randomZ;

		cc.GetInputContext().SetAimingAngles( angles );
		cc.GetInputContext().SetHeadingAngle( angles[0] );
	}
	
	void RegisterProjectile(IEntity projectile)
	{
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		if (!player)
			return;
		
		GC_ProjectileComponent projComp = GC_ProjectileComponent.Cast(projectile.FindComponent(GC_ProjectileComponent));
		if (!projComp)
			return;
		
		if (m_aProjectiles.Contains(projComp))
			return;
		
		if (!projComp.IsEnabled())
			return;
		
		ProjectileMoveComponent moveComp = ProjectileMoveComponent.Cast(projectile.FindComponent(ProjectileMoveComponent));
		if (!moveComp)
			return;

		projComp.entity = projectile;
		projComp.move = moveComp;
		projComp.position = projectile.GetOrigin();
		
		m_aProjectiles.Insert(projComp);
		
		//Print("GC | Projectile Registered: " + projectile);
	}
	
	void UnregisterProjectile(GC_ProjectileComponent projectile)
	{
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		

		if (player && projectile.entity)
		{
			SCR_ChimeraCharacter cc = SCR_ChimeraCharacter.Cast(player);
			vector playerEyePos = cc.EyePosition();
			
			vector projPos = projectile.entity.GetOrigin();
			float dist = vector.Distance(projPos, playerEyePos);
		
			if (dist <= m_fMaxRange)
			{
				float multi = 1.5;
				if(IsInCover(projectile))
					multi -= m_fCoverMultiplier;
				
				AddSuppression(projectile, dist, multi);
			}
	
			//Print("GC | UnregisterProjectile: " + projectile.GetOwner());
		}
		
		m_aProjectiles.RemoveItem(projectile);
	}
	
	//Player is consider in cover if they can't be hit by a bullet
	protected bool IsInCover(GC_ProjectileComponent projectile)
	{
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		if (!player)
			return false;
		
		SCR_ChimeraCharacter cc = SCR_ChimeraCharacter.Cast(player);
		vector playerEyePos = cc.EyePosition();
		
		if (!projectile)
			return false;
		
		if (!projectile.move)
			return false;
		
		vector velocity = projectile.move.GetVelocity();
		if (velocity.LengthSq() <= 0.0001)
			return false;
		
		// Direction FROM player TOWARD shooter (opposite bullet direction)
		vector dir = -velocity;
		dir.Normalize();
		
		vector start = playerEyePos;
		vector end = start + dir * m_fCoverTraceLength;
		
		// Trace from eyes toward where the bullet came from
		TraceParam tp = MakeTraceParam(start, end, TraceFlags.ENTS | TraceFlags.WORLD);
		tp.Exclude = player;
		
		float fraction = GetGame().GetWorld().TraceMove(tp);
		
		m_shapes.Insert(Shape.Create(ShapeType.LINE, Color.RED, ShapeFlags.DEFAULT, start, end));
		//PrintFormat("GC | IsInCover: %1 - %2", fraction, tp.ColliderName);
		
		return fraction < 1.0;
	}

	protected vector GetFlybyPoint(vector currPos, vector prevPos, vector playerPos)
	{
		vector seg = currPos - prevPos;
		float segLenSq = vector.DistanceSq(prevPos, currPos);
	
		if (segLenSq <= 0.000001)
			return prevPos;
	
		vector toPlayer = playerPos - prevPos;
	
		float t = vector.Dot(toPlayer, seg) / segLenSq;
		t = Math.Clamp(t, 0.0, 1.0);
	
		vector closestPoint = prevPos + seg * t;
		
		return closestPoint;
	}

	float GetAmount()
	{
		return m_fSuppression;
	}
	
	float GetMaxRange()
	{
		return m_fMaxRange;
	}
	
	float GetMinRange()
	{
		return m_fMaxRange;
	}
	
	//! Debug shapes, remove later
	protected ref array<ref Shape> m_shapes = {};
	protected void CreateDebugCircle(vector position, int color = Color.RED, bool clear = false)
	{
		if (clear)
			m_shapes.Clear();
		
		m_shapes.Insert(Shape.CreateSphere(color, ShapeFlags.DEFAULT, position, 0.2));
	}
}
