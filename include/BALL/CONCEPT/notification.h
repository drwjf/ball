// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//
// $Id: notification.h,v 1.22 2005/07/16 21:00:29 oliver Exp $
//

#ifndef BALL_CONCEPT_NOTIFICATION_H
#define BALL_CONCEPT_NOTIFICATION_H

#ifndef BALL_COMMON_GLOBAL_H
#	include <BALL/COMMON/global.h>
#endif

#ifndef BALL_COMMON_RTTI_H
#	include <BALL/COMMON/rtti.h>
#endif

#define BALL_NOTIFICATION_MANAGER_GROWTH_THRESHOLD(slots)   (2 * (slots) - 1)

namespace BALL 
{

	/**	@name	Notification Management.
			Using the notification management, objects may exchange data and notify
			each other of events by invoking methods of the other object (remotely similar to the
			signal/slot mechanism of QT).
			 \par
			
			\ingroup ConceptsMiscellaneous
	*/
	//@{
	
	class NotificationManager_;
	class NotificationSource_;

	///
	class NotificationTarget_
	{
		friend class NotificationManager_;
		friend class NotificationSource_;

		public:

		///
		~NotificationTarget_()
			;

		///
		NotificationTarget_* getNextTarget()
			;

		///
		void* getTarget()
			;

		private:

		///
		NotificationTarget_()
			;

		///
		NotificationTarget_(NotificationTarget_* notification_target, void* target)
			;


		NotificationTarget_*	next_;
		void*									target_;
	};

	///
	class NotificationSource_
	{
		friend class NotificationManager_;

		public:

		///
		~NotificationSource_()
			;

		///
		void enable()
			;
	
		///
		void disable()
			;

		///
		bool isEnabled()
			;
	
		///
		bool isDisabled()
			;

	
		private:

		///
		NotificationSource_()
			;

		///
		NotificationSource_(NotificationSource_* notification_source, void* source)
			;

		///
		void destroy()
			;

		NotificationSource_*	next_;
		NotificationTarget_*	first_;
		void*									source_;
		Size									number_of_targets_;
		bool									enabled_;
	};

	///
	class NotificationSlot_
	{
		friend class NotificationManager_;

		public:

		///
		~NotificationSlot_()
			;


		private:

		NotificationSlot_()
			;

		NotificationSource_* first_;
	};

	///
	class NotificationManager_
	{
		friend NotificationManager_& NotificationManager()
			;

		public:

		///
		static const Size INITIAL_NUMBER_OF_SLOTS;

		///
		~NotificationManager_()
			;

		///
		Size getSize()
			;

		///
		void insert(void* source, void* target)
			;

		///
		void remove(void* source)
			;

		///
		void remove(void* source, void* target)
			;

		///
		NotificationSource_* findSource(void* source)
			;

		///
		NotificationTarget_* findFirstTarget(void* source)
			;

		///
		NotificationTarget_* findTarget(void* source, void* target)
			;

		///
		NotificationSource_* findEnabledSource(void* source)
			;

		///
		NotificationTarget_* findEnabledFirstTarget(void* source)
			;

		///
		NotificationTarget_* findEnabledTarget(void* source, void* target)
			;

		///
		void destroy()
			;

		///
		void enable()
			;

		///
		void disable()
			;

		///
		bool isEnabled()
			;
	
		///
		bool isDisabled()
			;
	
		///
		bool isEmpty()
			;

		///
		bool isInserted(void* source)
			;

		///
		bool isInserted(void* source, void* target)
			;

		///
		void dump(std::ostream& s)
			;


		private:

		NotificationManager_()
			;
			
		bool needResize_()
			;

		void resize_()
			;

		Index hash_(void* ptr)
			;

		Size 								number_of_targets_;
		Size 								number_of_slots_;
		NotificationSlot_* 	slots_;
		bool 								enabled_;
	};

	///	
  NotificationManager_& NotificationManager()
		;
 
	///
	void NotificationManagerEnable()
		;

	///
	void NotificationManagerDisable()
		;

	///
	bool NotificationManagerIsEnabled()
		;

	///
	bool NotificationManagerIsDisabled()
		;

	template <class NotificationSource>
	inline void NotificationSourceEnable(NotificationSource &notification_source)
		
	{
		NotificationSource_* source_ptr = NotificationManager().findSource((void*)&notification_source);

		if (source_ptr != 0)
		{
			source_ptr->enable();
		}
	}

	template <class NotificationSource>
	inline void NotificationSourceDisable(NotificationSource &notification_source)
		
	{
		NotificationSource_* source_ptr = NotificationManager().findSource((void*)&notification_source);

		if (source_ptr != 0)
		{
			source_ptr->disable();
		}
	}

	template <class NotificationSource>
	inline bool NotificationSourceIsEnabled(NotificationSource &notification_source)
		
	{
		return (NotificationManager().findEnabledSource((void*)&notification_source) != 0);
	}

	template <class NotificationSource>
	inline bool NotificationSourceIsDisabled(NotificationSource &notification_source)
		
	{
		return (NotificationManager().findEnabledSource((void*)&notification_source) == 0);
	}


	/**	Notification Target Class.
	*/
	template <class NotificationSource>
	class NotificationTarget
	{
		public:
		virtual ~NotificationTarget()  {}

		/**	The method to be invoked upon receiving a notification
		*/
		virtual bool onNotify(NotificationSource& source) = 0;

		/** 
		*/
		virtual unsigned short countNotificationTypes_()
			
		{
			return 0;
		}

		/**
		*/
		static void* getNotificationType_()
			
		{
			return RTTI::getClassID<NotificationSource>();
		}

		/**
		*/
		virtual void* getVirtualNotificationType_()
			
		{
			return getNotificationType_();
		}
	};

	///
	template <class NotificationSource, class T1>
	class NotificationTarget1
	{
		public:

		///	
		virtual bool onNotify(NotificationSource &, T1) = 0;

		///
		virtual unsigned short countNotificationTypes_()
			
		{
			return 1;
		}

		///
		static void* getNotificationType_()
			
		{
			return (void* )&getNotificationType_;
		}

		///
		virtual void* getVirtualNotificationType()
			
		{
			return getNotificationType_();
		}
	};

	///
	template <class NotificationSource, class T1, class T2>
	class NotificationTarget2
	{
		public:

		///
		virtual bool onNotify(NotificationSource &, T1, T2) = 0;

		///
		virtual unsigned short countNotificationTypes_()
			
		{
			return 2;
		}

		///
		static void* getNotificationType_()
			
		{
			return (void* )&getNotificationType_;
		}

		///
		virtual void* getVirtualNotificationType_()
			
		{
			return getNotificationType_();
		}
	};



	template <class Source, class Target>
	inline void NotificationRegister(const Source &source, const Target &target)
		
	{
		NotificationManager().insert((void*)&source, (void*)&(NotificationTarget<Source> &)target);
	}

	template <class Source, class Target, class T1>
	inline void NotificationRegister1(const Source& source, const Target& target, const T1 &)
		
	{
		NotificationManager().insert((void*)&source, (void*)&(NotificationTarget1<Source, T1> &)target);
	}

	template <class NotificationSource, class NotificationTarget, class T1, class T2>
	inline void NotificationRegister2
		(const NotificationSource &notification_source,
		 const NotificationTarget &notification_target,
		 const T1 &, const T2 &)
		
	{
		NotificationManager().insert
			((void*)&notification_source, 
			 (void*)&(NotificationTarget2<NotificationSource, T1, T2> &)notification_target);
	}

	void NotificationUnregisterAll()
		;

	template <class NotificationSource>
	inline void NotificationUnregister(const NotificationSource &notification_source)
		
	{
		NotificationManager().remove((void*)&notification_source);
	}

	template <class Source, class Target>
	inline void NotificationUnregister(const Source &source, const Target &target)
		
	{
		NotificationManager().remove((void*)&source, (void*)&(NotificationTarget<Source> &)target);
	}

	template <class NotificationSource, class NotificationTarget, class T1>
	inline void NotificationUnregister1
		(const NotificationSource &notification_source,
		 const NotificationTarget &notification_target,
		 const T1 &)
		
	{
		NotificationManager().remove((void*)&notification_source, 
			 (void*)&(NotificationTarget1<NotificationSource, T1> &)notification_target);
	}

	template <class NotificationSource, class NotificationTarget, class T1, class T2>
	inline void	NotificationUnregister2
		(const NotificationSource &notification_source,
		 const NotificationTarget &notification_target,
		 const T1& , const T2& )
		
	{
		NotificationManager().remove
			((void*)&notification_source, 
			 (void*)&(NotificationTarget2<NotificationSource, T1, T2> &)notification_target);
	}

	template <class NotificationSource>
	inline bool NotificationIsRegistered(const NotificationSource &notification_source)
		
	{
		return NotificationManager().isInserted((void*)&notification_source);
	}

	template <class Source, class Target>
	inline bool NotificationIsRegistered(const Source& source, const Target& target)
		
	{
		return NotificationManager().isInserted((void*)&source, 
			 (void*)&(NotificationTarget<Source> &)target);
	}

	template <class NotificationSource>
	bool Notify(NotificationSource &notification_source)
		
	{
		if (NotificationManager().isDisabled() == true)
		{
			return true;
		}

		NotificationTarget<NotificationSource>* notification_target0;

		for (NotificationTarget_* notification_target 
					= NotificationManager().findEnabledFirstTarget((void*)&notification_source);
				 notification_target != 0;
				 notification_target = notification_target->getNextTarget())
		{
			notification_target0 = (NotificationTarget<NotificationSource>* )notification_target->getTarget();

			if (notification_target0 != 0 && notification_target0->countNotificationTypes_() == 0
					&& notification_target0->getVirtualNotificationType_() 
						== NotificationTarget<NotificationSource>::getNotificationType_())
			{
				if (notification_target0->onNotify(notification_source) == false)
				{
					return false;
				}
			}
		}

		return true;
	}

	template <class NotificationSource, class T1>
	bool Notify(NotificationSource &notification_source, const T1 &data1)
		
	{
		if (NotificationManager().isDisabled() == true)
		{
			return true;
		}
		
		NotificationTarget1<NotificationSource, T1>* notification_target1;

		for (NotificationTarget_* notification_target 
		 = NotificationManager().findEnabledFirstTarget((void*)&notification_source);
				 notification_target != 0;
				 notification_target = notification_target->getNextTarget())
		{
			notification_target1
				= (NotificationTarget1<NotificationSource, T1>* )notification_target->getTarget();

			if (notification_target1 != 0
					&& notification_target1->countNotificationTypes_() == 1
					&& notification_target1->getVirtualNotificationType_() 
						 == NotificationTarget1<NotificationSource, T1>::getNotificationType_())
			{
				if ((*notification_target1).onNotify(notification_source, data1) == false)
				{
					return false;
				}
			}
		}

		return true;
	}

	template <class NotificationSource, class T1, class T2>
	bool Notify (NotificationSource& notification_source, const T1& data1, const T2& data2)
		
	{
		if (NotificationManager().isDisabled())
		{
			return true;
		}
		
		NotificationTarget2<NotificationSource, T1, T2>* notification_target2;

		for (NotificationTarget_* notification_target 
					 = NotificationManager().findEnabledFirstTarget((void*)&notification_source);
				 notification_target != 0;
				 notification_target = notification_target->getNextTarget())
		{
			notification_target2 = (NotificationTarget2<NotificationSource, T1, T2>* )notification_target->getTarget();

			if (notification_target2 != 0
					&& notification_target2->countNotificationTypes_() == 2
					&& notification_target2->getVirtualNotificationType_() 
					 == NotificationTarget2<NotificationSource, T1, T2>::getNotificationType_())
			{
				if ((*notification_target2).onNotify(notification_source, data1, data2) == false)
				{
					return false;
				}
			}
		}

		return true;
	}

	//@}
} // namespace BALL

#endif // BALL_CONCEPT_NOTIFICATION_H