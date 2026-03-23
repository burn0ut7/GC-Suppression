class GC_SuppressionSystem : GameSystem
{
	[Attribute("10", UIWidgets.Auto,
	"Flyby outer radius (meters). Suppression from a passing bullet fades to 0 at this distance (measured from player eye to closest approach).",
	params: "0 inf", category: "Projectile")]
	protected float m_fMaxRange;
	
	[Attribute("10", UIWidgets.Auto,
		"Distance from source (meters). Inside this distance, suppression is not applied.",
		params: "0 inf", category: "Projectile")]
	protected float m_fMinRange;
	
	[Attribute("0.5", UIWidgets.Auto,
		"Flinch radius (meters). Triggers flinch when a bullet's closest approach is within this distance.",
		params: "0 inf", category: "Projectile")]
	protected float m_fFlinchRange;
	
	[Attribute("1", UIWidgets.Auto,
		"Base screen-shake strength on flinch. 0 disables shake.",
		params: "0 inf", category: "Projectile")]
	protected float m_fFlinchShakeMultiplier;
	
	[Attribute("0.5", UIWidgets.Auto,
		"Extra flinch shake scaling from current suppression. At full suppression, shake is multiplied by (1 + this).",
		params: "0 inf", category: "Projectile")]
	protected float m_fFlinchShakeSuppressedMultiplier;
	
	[Attribute("2.0", UIWidgets.Auto,
		"Mass contribution scale. Suppression mass term = ProjectileMass * this.",
		params: "0 inf", category: "Projectile")]
	protected float m_fMassMultiplier;
	
	[Attribute("0.001", UIWidgets.Auto,
		"Speed contribution scale. Suppression speed term = ProjectileSpeed * this.",
		params: "0 inf", category: "Projectile")]
	protected float m_fSpeedMultiplier;
	
	[Attribute("0.04", UIWidgets.Auto,
		"Global scaling applied to computed flyby suppression (after mass/speed/distance).",
		params: "0 inf", category: "Projectile")]
	protected float m_fBaseSuppressionMultiplier;
	
	[Attribute("1", UIWidgets.Auto,
		"Impact suppression multiplier. Applied when a bullet impacts near the player (stronger than flyby).",
		params: "0 inf", category: "Projectile")]
	protected float m_fHitSuppressionMultiplier;
	
	[Attribute("5.0", UIWidgets.Auto,
		"Cover check trace length (meters). Ray is cast from player eyes toward the bullet source direction to detect blocking cover.",
		params: "0 inf", category: "Suppression")]
	protected float m_fCoverTraceLength;
	
	[Attribute("7.5", UIWidgets.Auto,
		"Recovery delay (seconds). Time without new suppression before suppression starts to decay.",
		params: "0 inf", category: "Suppression")]
	protected float m_fRecoveryDelay;
	
	[Attribute("0.2", UIWidgets.Auto,
		"Recovery rate (suppression units per second). 0.2 means suppression drops by 20% per second once recovery starts.",
		params: "0 1", category: "Suppression")]
	protected float m_fRecoveryRate;
	
	[Attribute("0.5", UIWidgets.Auto,
		"Cover suppression reduction (0–1). Subtracted from the suppression multiplier when the player is considered in cover (0.5 = 50% less).",
		params: "0 1", category: "Suppression")]
	protected float m_fCoverMultiplier;
	
	[Attribute("1.0", UIWidgets.Auto, "Multiplier applied to explosive payload strength before impulse is calculated.", params: "0 inf", category: "Explosion")]
	protected float m_fExplosionPayloadMultiplier;
	
	[Attribute("1.0", UIWidgets.Auto, "Multiplier applied to explosion distance before falloff. Higher values make the impulse weaker at range.", params: "0.001 inf", category: "Explosion")]
	protected float m_fExplosionDistanceMultiplier;
	
	[Attribute("1.0", UIWidgets.Auto, "Multiplier applied to the final calculated explosion impulse.", params: "0 inf", category: "Explosion")]
	protected float m_fExplosionImpulseMultiplier;
	
	[Attribute("1.5", UIWidgets.Auto, "Power used for explosion distance falloff. Lower values carry impulse farther, higher values drop it faster.", params: "0.001 inf", category: "Explosion")]
	protected float m_fExplosionDistanceFalloffPower;
	
	[Attribute("1.0", UIWidgets.Auto, "Impulse at which explosion suppression starts becoming noticeable.", params: "0 inf", category: "Explosion")]
	protected float m_fExplosionSuppressionStartImpulse;
	
	[Attribute("45", UIWidgets.Auto, "Impulse that counts as extreme and results in full suppression.", params: "0.001 inf", category: "Explosion")]
	protected float m_fExplosionSuppressionFullImpulse;
	
	[Attribute("1.0", UIWidgets.Auto, "Controls how quickly suppression ramps from start impulse to full impulse. Higher values keep low impulses weaker for longer.", params: "0.001 inf", category: "Explosion")]
	protected float m_fExplosionSuppressionPower;
	
	[Attribute("1", UIWidgets.CheckBox, "Enable explosion ragdolling when the player is hit by a strong enough blast.", category: "Explosion")]
	protected bool m_bEnableExplosionRagdoll;
	
	[Attribute("35.0", UIWidgets.Auto, "Minimum explosion impulse required to ragdoll the player.", params: "0 inf", category: "Explosion")]
	protected float m_fExplosionRagdollMinImpulse;
	
	[Attribute("120.0", UIWidgets.Auto, "Explosion impulse that results in maximum ragdoll throw force.", params: "0.001 inf", category: "Explosion")]
	protected float m_fExplosionRagdollMaxImpulse;
	
	[Attribute("350.0", UIWidgets.Auto, "Minimum impulse force applied to the ragdolled player.", params: "0 inf", category: "Explosion")]
	protected float m_fExplosionRagdollMinForce;
	
	[Attribute("1200.0", UIWidgets.Auto, "Maximum impulse force applied to the ragdolled player.", params: "0 inf", category: "Explosion")]
	protected float m_fExplosionRagdollMaxForce;
	
	[Attribute("0.35", UIWidgets.Auto, "Extra upward force ratio added to explosion ragdoll impulse.", params: "0 inf", category: "Explosion")]
	protected float m_fExplosionRagdollUpForceMultiplier;
	
	// Projectiles tracked by the system
	protected ref array<GC_ProjectileComponent> m_aProjectiles = {};
	
	// Player suppression value 0-1
	protected float m_fSuppression = 0;
	
	protected int m_iLastUpdateMs = 0;
	protected int m_iLastSuppressionMs = 0;
	
	protected bool m_bIsEnabled = true;
	
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
		
		GetGame().GetInputManager().AddActionListener("CharacterMelee", EActionTrigger.DOWN, Test1);
	}
	
	void Test1()
	{
		Print("GC | Test1");
		RagdollPlayer(150, "0 0 0");

	}
	
	void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		SetEnabled(true);
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
		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());
		if (!player)
			return;
	
		vector playerEyePos = player.EyePosition();
	
		float maxRangeSq = m_fMaxRange * m_fMaxRange;
		float flinchRangeSq = m_fFlinchRange * m_fFlinchRange;
	
		bool isInVehicle = player.IsInVehicle();
	
		for (int i = m_aProjectiles.Count() - 1; i >= 0; i--)
		{
			GC_ProjectileComponent projectile = m_aProjectiles[i];
	
			bool removeProjectile = false;
	
			IEntity owner = projectile.GetOwner();
			if (!owner)
				removeProjectile = true;
			else
			{
				ProjectileMoveComponent move = ProjectileMoveComponent.Cast(owner.FindComponent(ProjectileMoveComponent));
				if (!move)
					removeProjectile = true;
				else
				{
					vector projPos = owner.GetOrigin();

					float approach = vector.Dot(playerEyePos - projPos, move.GetVelocity());
	
					if (approach > 0)
					{
						projectile.position = projPos;
					}
					else
					{
						removeProjectile = true;

						vector flybyPos = GetFlybyPoint(projPos, projectile.position, playerEyePos);
						float distSq = vector.DistanceSq(flybyPos, playerEyePos);
	
						bool inSuppRange = distSq <= maxRangeSq;
	
						if (inSuppRange)
						{
							bool doEffects = true;
	
							if (isInVehicle)
							{
								if (!IsLineOfSight(flybyPos))
									doEffects = false;
							}
	
							if (doEffects)
							{
								if (inSuppRange)
								{
									bool isInCover = IsInCover(projectile);
	
									float dist = Math.Sqrt(distSq);
	
									float multi = 1.0;
									if (isInCover)
										multi = Math.Clamp(multi - m_fCoverMultiplier, 0.0, 1.0);
	
									float suppression = GetBulletSuppression(projectile, dist, multi);
									AddSuppression(suppression);
								}
	
								if (distSq <= flinchRangeSq)
									Flinch();
							}
						}
					}
				}
			}
	
			if (removeProjectile)
				m_aProjectiles.Remove(i);
		}
	}
	
	protected void AddSuppression(float suppression)
	{
		if (suppression <= 0)
			return;
		
		m_fSuppression = Math.Clamp(m_fSuppression + suppression, 0, 1);
		
		m_iLastSuppressionMs = System.GetTickCount();
		
		UpdateEffect();
	}
	
	protected float GetBulletSuppression(GC_ProjectileComponent projectile, float distance, float multiplier = 1)
	{
		IEntity projEntity = projectile.GetOwner();
		if(!projEntity)
			return 0;
		
		ProjectileMoveComponent move = ProjectileMoveComponent.Cast(projEntity.FindComponent(ProjectileMoveComponent));
		if(!move)
			return 0;

		BaseContainer container = move.GetComponentSource(projEntity);
		if(!container)
			return 0;
		
		float mass;
		container.Get("Mass", mass);
		if (!mass)
			return 0;
		
		float speed = move.GetVelocity().Length();
		
		float distanceTerm = 0.0;
		if (distance <= 0.0)
			distanceTerm = 1.0;
		else if (distance < m_fMaxRange)
			distanceTerm = 1.0 - (distance / m_fMaxRange);

		float suppression = (mass * m_fMassMultiplier);
		suppression += (speed * m_fSpeedMultiplier);
		suppression *= distanceTerm;
	
		suppression = suppression * m_fBaseSuppressionMultiplier * multiplier;
		
		suppression = Math.Clamp(suppression, 0, 1);
		
		return suppression;
	}
	
	protected float GetBulletSuppression(IEntity bullet, float distance, float multiplier = 1)
	{
		ProjectileMoveComponent move = ProjectileMoveComponent.Cast(bullet.FindComponent(ProjectileMoveComponent));
		if (!move)
			return 0;
		
		BaseContainer container = move.GetComponentSource(bullet);
		if(!container)
			return 0;
		
		float mass;
		container.Get("Mass", mass);
		if (!mass)
			return 0;
		
		float speed = move.GetVelocity().Length();
		
		float distanceTerm = 0.0;
		if (distance <= 0.0)
			distanceTerm = 1.0;
		else if (distance < m_fMaxRange)
			distanceTerm = 1.0 - (distance / m_fMaxRange);

		float suppression = (mass * m_fMassMultiplier);
		suppression += (speed * m_fSpeedMultiplier);
		suppression *= distanceTerm;
	
		suppression = suppression * m_fBaseSuppressionMultiplier * multiplier;
		
		suppression = Math.Clamp(suppression, 0, 1);
		
		return suppression;
	}
	
	protected float GetExplosionSuppression(float impulse, float multiplier = 1)
	{
		if (impulse <= 0)
			return 0;
	
		if (multiplier <= 0)
			return 0;
	
		float startImpulse = m_fExplosionSuppressionStartImpulse;
		if (startImpulse < 0)
			startImpulse = 0;
	
		float fullImpulse = m_fExplosionSuppressionFullImpulse;
		if (fullImpulse <= startImpulse)
			fullImpulse = startImpulse + 0.001;
	
		if (impulse <= startImpulse)
			return 0;
	
		float normalized = Math.InverseLerp(startImpulse, fullImpulse, impulse);
		normalized = Math.Clamp(normalized, 0.0, 1.0);
	
		float power = m_fExplosionSuppressionPower;
		if (power <= 0)
			power = 1.0;
	
		float suppression = Math.Pow(normalized, power);
		suppression = suppression * multiplier;
	
		PrintFormat(
			"GC | ExplosionSuppression | impulse:%1 start:%2 full:%3 normalized:%4 power:%5 suppression:%6",
			impulse,
			startImpulse,
			fullImpulse,
			normalized,
			power,
			suppression
		);
		
		return Math.Clamp(suppression, 0.0, 1.0);
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
		
		if (m_fFlinchShakeMultiplier > 0)
		{
			float suppressionMulti = 1.0 + m_fSuppression * m_fFlinchShakeSuppressedMultiplier;
	
			float magnitude = m_fFlinchShakeMultiplier * suppressionMulti;
			
			ScreenShake(magnitude);
		}
	}
	
	protected void ScreenShake(float magnitude = 1, float inTime = 0.01, float sustainTime = 0.1, float outTime = 0.24)
	{
		CameraBase camera = GetGame().GetCameraManager().CurrentCamera();
		SCR_CameraShakeManagerComponent.AddCameraShake(magnitude, magnitude, inTime, sustainTime, outTime);
	}
	
	void HandleExplosion(IEntity source, IEntity explosion, vector transform[3])
	{
		PrintFormat("GC | HandleExplosion: %1", explosion);
		
		float impulse = GetExplosiveImpulse(explosion);
		
		float suppression = GetExplosionSuppression(impulse);
		//if(suppression < 0.01)
		//	return;
		
		//Cover check
		float multiplier = 1.0;
		bool isLOS = IsLineOfSight(transform, {source});
		if(!isLOS)
			multiplier = 0.5;

		//ragdoll
		
		
		
		//Screen shake
		
		AddSuppression(suppression);
	}
	
	//position/direction/normal
	protected void RagdollPlayer(float impulse, vector direction)
	{
		if (!m_bEnableExplosionRagdoll)
			return;
	
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		if (!player)
			return;
	
		if (impulse < m_fExplosionRagdollMinImpulse)
			return;
	
		SCR_CharacterControllerComponent cc = SCR_CharacterControllerComponent.Cast(player.FindComponent(SCR_CharacterControllerComponent));
		if (!cc)
			return;
	
		float maxImpulse = m_fExplosionRagdollMaxImpulse;
		if (maxImpulse <= m_fExplosionRagdollMinImpulse)
			maxImpulse = m_fExplosionRagdollMinImpulse + 0.001;
	
		float normalized = Math.InverseLerp(m_fExplosionRagdollMinImpulse, maxImpulse, impulse);
		normalized = Math.Clamp(normalized, 0.0, 1.0);
	
		float throwForce = Math.Lerp(m_fExplosionRagdollMinForce, m_fExplosionRagdollMaxForce, normalized);
	
		vector throwDirection = direction.Normalized();
		if (throwDirection == vector.Zero)
			throwDirection = vector.Up;
	
		throwDirection[1] = throwDirection[1] + m_fExplosionRagdollUpForceMultiplier;
		throwDirection = throwDirection.Normalized();
	
		vector throwImpulse = throwDirection * throwForce;
	
		cc.Ragdoll();
		cc.RefreshRagdoll(1);
		player.Update();
	
		Physics physics = player.GetPhysics();
		if (!physics)
			return;
	
		physics.ClearForces();
		physics.ApplyImpulse(throwImpulse);
	
		PrintFormat(
			"GC | ExplosionRagdoll | impulse:%1 normalized:%2 throwForce:%3 direction:%4 throwImpulse:%5",
			impulse,
			normalized,
			throwForce,
			throwDirection,
			throwImpulse
		);
	}
	
	//(m_eSuppType, source, transform, speed);
	void HandleBulletImpact(IEntity bullet, vector transform[3], float distance, float speed)
	{
		if (!m_bIsEnabled)
			return;
		
		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());
		if (!player)
			return;
		
		bool isInCover = IsInCover(transform[1]);
		
		if(player.IsInVehicle() && !isInCover)
		{
			if(!IsLineOfSight(transform, null))
				return;
		}
		
		float multi = m_fHitSuppressionMultiplier;
		if(isInCover)
			multi -= m_fCoverMultiplier;
		
		float suppression = GetBulletSuppression(bullet, distance, multi);
		AddSuppression(suppression);
		
		//PrintFormat("GC | HandleBulletImpact: %1 - %2 - %3", bullet, distance, speed);
	}
	
	void RegisterProjectile(IEntity projectile)
	{
		if (!m_bIsEnabled)
			return;
		
		IEntity player = GetGame().GetPlayerController().GetControlledEntity();
		if (!player)
			return;
		
		GC_ProjectileComponent projComp = GC_ProjectileComponent.Cast(projectile.FindComponent(GC_ProjectileComponent));
		if (!projComp)
			return;
		
		if (m_aProjectiles.Contains(projComp))
			return;
		
		ProjectileMoveComponent moveComp = ProjectileMoveComponent.Cast(projectile.FindComponent(ProjectileMoveComponent));
		if (!moveComp)
			return;

		projComp.position = projectile.GetOrigin();
		
		//PrintFormat("GC | RegisterProjectile: %1", projComp);
		m_aProjectiles.Insert(projComp);
	}
	
	void UnregisterProjectile(GC_ProjectileComponent projectile)
	{
		if(m_aProjectiles.Contains(projectile))
			m_aProjectiles.RemoveItem(projectile);
	}
	
	protected bool IsInCover(vector direction)
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return false;
		
		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(pc.GetControlledEntity());
		if (!player)
			return false;

		vector start = player.EyePosition();
		vector end = start - direction * m_fCoverTraceLength;

		array<IEntity> excluded = GetAllRelated(player);
		
		// Trace from eyes toward where the bullet came from
		TraceParam tp = MakeTraceParam(start, end, TraceFlags.ENTS | TraceFlags.WORLD);
		tp.ExcludeArray = excluded;
		
		float fraction = GetGame().GetWorld().TraceMove(tp);
		
		return fraction < 1.0;
	}
	
	//Player is consider in cover if they can't be hit by a bullet
	protected bool IsInCover(GC_ProjectileComponent projectile)
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return false;
		
		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(pc.GetControlledEntity());
		if (!player)
			return false;

		vector playerEyePos = player.EyePosition();
		
		if (!projectile)
			return false;
		
		ProjectileMoveComponent move = ProjectileMoveComponent.Cast(projectile.GetOwner().FindComponent(ProjectileMoveComponent));
		if(!move)
			return false;
		
		vector velocity = move.GetVelocity();
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
		
		return fraction < 1.0;
	}
	
	protected bool IsLineOfSight(vector position)
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return false;
		
		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(pc.GetControlledEntity());
		if (!player)
			return false;
		
		array<IEntity> excluded = GetAllRelated(player);
		
		TraceParam tp = MakeTraceParam(player.EyePosition(), position, TraceFlags.ENTS | TraceFlags.WORLD);
		tp.ExcludeArray = excluded;
		
		float fraction = GetGame().GetWorld().TraceMove(tp);

		return fraction == 1.0;
	}
	
	protected bool IsLineOfSight(vector transform[3], array<IEntity> exclusions)
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return false;
	
		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(pc.GetControlledEntity());
		if (!player)
			return false;

		vector start = player.EyePosition();
	
		float epsilon = 0.01;
		vector endPos = transform[0] + transform[2] * epsilon;
		array<IEntity> excluded = GetAllRelated(player);
		
		if(exclusions)
			excluded.InsertAll(exclusions);
		
		TraceParam tp = MakeTraceParam(start, endPos, TraceFlags.ENTS | TraceFlags.WORLD);
		tp.ExcludeArray = excluded;
	
		float fraction = GetGame().GetWorld().TraceMove(tp);
		
		if(fraction == 1)
			CreateDebugLine(start, endPos, Color.GREEN);
		else
			CreateDebugLine(start, endPos);
		
		//PrintFormat("GC | IsLineOfSight | Hit:%1", tp.TraceEnt);
		
		return fraction == 1.0;
	}
	
	protected float GetExplosivePayload(IEntity explosion)
	{
		BaseTriggerComponent trigger = BaseTriggerComponent.Cast(explosion.FindComponent(BaseTriggerComponent));
		if(!trigger)
			return 0;
		
		BaseContainer container = trigger.GetComponentSource(explosion);
		BaseContainerList effects = container.GetObjectArray("PROJECTILE_EFFECTS");
		container = null;
		for (int i = 0, num = effects.Count(); i < num; i++)
		{
			BaseContainer cont = effects.Get(i);
			if (cont.GetClassName() == "ExplosionDamageContainer")
				container = cont;
		}
		
		if(!container)
			return 0;
		
		float payload, tntEquivalent;
		container.Get("ChargeWeight", payload);
		container.Get("TntEquivalent", tntEquivalent);
		
		return payload * tntEquivalent;
	}
	
	protected float GetExplosiveImpulse(IEntity explosion)
	{
		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());
		if (!player)
			return 0;
	
		float payload = GetExplosivePayload(explosion);
		if (payload <= 0)
			return 0;
	
		float distance = vector.Distance(explosion.GetOrigin(), player.EyePosition());
		float scaledPayload = payload * m_fExplosionPayloadMultiplier;
		if (scaledPayload <= 0)
			return 0;
	
		float scaledDistance = distance * m_fExplosionDistanceMultiplier;
		if (scaledDistance <= 0)
			return 0;
	
		float falloffPower = m_fExplosionDistanceFalloffPower;
		if (falloffPower <= 0)
			falloffPower = 1.5;
	
		float attenuation = 1.0 / Math.Pow(scaledDistance, falloffPower);
		float impulse = scaledPayload * attenuation;
		impulse = impulse * m_fExplosionImpulseMultiplier;
	
		PrintFormat("GC | payload:%1 distance:%2 attenuation:%3 impulse:%4", scaledPayload, scaledDistance, attenuation, impulse);
		
		return Math.Max(impulse, 0);
	}
		
	protected array<IEntity> GetAllRelated(IEntity parent)
	{
		array<IEntity> childern = {parent};
	
		IEntity child = parent.GetChildren();
		while (child)
		{
			childern.Insert(child);
			child = child.GetSibling();
		}
	
		return childern;
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
		return m_fMinRange;
	}
	
	void SetEnabled(bool isEnabled)
	{

		SCR_ScreenEffectsManager sem = SCR_ScreenEffectsManager.GetScreenEffectsDisplay();
		if (sem)
		{
			GC_SuppressionScreenEffect sse = GC_SuppressionScreenEffect.Cast(sem.GetEffect(GC_SuppressionScreenEffect));
			if (sse)
			{
				sse.SetEnabled(isEnabled);
				sse.ClearEffects();
			}
		}

		m_fSuppression = 0;
		m_aProjectiles.Clear();
		
		m_bIsEnabled = isEnabled;
	}
	
	//! Debug shapes, remove later
	protected ref array<ref Shape> m_shapes = {};
	protected void CreateDebugCircle(vector position, int color = Color.RED, bool clear = false)
	{
		if (clear)
			m_shapes.Clear();
		
		m_shapes.Insert(Shape.CreateSphere(color, ShapeFlags.DEFAULT, position, 0.05));
	}
	
	protected void CreateDebugLine(vector start, vector end, int color = Color.RED, bool clear = false)
	{
		if (clear)
			m_shapes.Clear();
		
		m_shapes.Insert(Shape.Create(ShapeType.LINE, color, ShapeFlags.DEFAULT, start, end));
	}
}
