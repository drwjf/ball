#ifndef BALL_STRUCTURE_DISULFIDBONDPROCESSOR_H
#define BALL_STRUCTURE_DISULFIDBONDPROCESSOR_H

#ifndef BALL_CONCEPT_PROCESSOR_H
# include <BALL/CONCEPT/processor.h>
#endif

#ifndef BALL_KERNEL_SYSTEM_H
# include <BALL/KERNEL/system.h>
#endif

namespace BALL
{
	/** Disulfid Bond Processor
	    \ingroup StructureMiscellaneous
	*/

	/** Detection and Computation of disulfid bonds of the atom container.
	 */
	class BALL_EXPORT DisulfidBondProcessor
		: public UnaryProcessor<AtomContainer>
	{
		public:
		/** @name Type definitions
		 */
		//@{
		typedef std::pair<Residue*, Residue*>    DisulfidBond;
		typedef	std::vector<DisulfidBond>  DisulfidBondVector;
		//@}

		BALL_CREATE(DisulfidBondProcessor);


		/** @name	Constructors and Destructors
		 */
		//@{

		/// Default Constructor
		DisulfidBondProcessor();

		/// Destructor
		virtual ~DisulfidBondProcessor();
		//@}

		/**	@name	Processor-related methods 
		*/
		//@{

		/// Processor method which is called before the operator()-call.
		virtual bool start();

		/** Clears the data structures.
		*/
		void clear();

		/** Operator () for the processor 
		 */
		virtual Processor::Result operator ()(AtomContainer& ac);

		/// Processor method which is called after the operator()-call.
		virtual bool finish();

		//@}

		/** @name Accessors.
		*/
		//@{

		/// Return the number of detected disulfid bonds.
		Size getNumberOfDetectedDisulfidBonds() {return sulfur_bridges_.size();};

		/// Return the vector of disulfid bonds.
		DisulfidBondVector& getDisulfidBonds() {return sulfur_bridges_;};

		/// Return the vector of disulfid bonds, const variant.
		const DisulfidBondVector& getDisulfidBonds() const {return sulfur_bridges_;};

		//@}

		/// Connect two sulfurs by disulid bond
		bool connect(Atom* atom1, Atom* atom2);

		/// Connect two residues by disulid bond
		bool connect(Residue* residue1, Residue* residue2);

		/// Connect two AtomContainers by disulid bond
		bool connect(Composite* composite1, Composite* composite2);

		protected:
			DisulfidBondVector  sulfur_bridges_;
	};
}

#endif // BALL_STRUCTURE_DISULFIDBONDPROCESSOR_H