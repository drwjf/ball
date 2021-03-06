// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//

#include <BALL/FORMAT/HINFile.h>
#include <BALL/CONCEPT/composite.h>
#include <BALL/KERNEL/residue.h>
#include <BALL/KERNEL/system.h>
#include <BALL/KERNEL/protein.h>
#include <BALL/KERNEL/atom.h>
#include <BALL/KERNEL/PDBAtom.h>
#include <BALL/KERNEL/bond.h>
#include <BALL/KERNEL/PTE.h>

#include <stack>

namespace BALL
{

  struct HINFileBondStruct
  {
    Size        atom1;
    Size        atom2;
    Bond::Order order;
	};

	HINFile::HINFile()
		:	GenericMolFile(),
			box_(0.0, 0.0, 0.0, 0.0, 0.0, 0.0),
			temperature_(0.0)
	{
	}

	HINFile::HINFile(const String& name, File::OpenMode open_mode)
		: GenericMolFile(),
			box_(0.0, 0.0, 0.0, 0.0, 0.0, 0.0),
			temperature_(0.0)
	{
		open(name, open_mode);
	}


	HINFile::~HINFile()
	{
	}

	const HINFile& HINFile::operator = (const HINFile& rhs)
	{
		box_ = SimpleBox3(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
		temperature_ = 0.0;

		GenericMolFile::operator = (rhs);

		return *this;
	}

	void HINFile::writeAtom_(const Atom& atom, Size number, Size atom_offset)
	{
		getFileStream() << "atom " << number + 1 - atom_offset << " ";
		String name = atom.getName();
		if (name != "")
		{
			// if the atom name contains blanks or some bullshit like that, 
			// truncate it to the first field and complain about it
			if (name.countFields() > 1)
			{
				getFileStream() << name.getField(0) <<  " ";
				Log.warn() << "HINFile::write: truncated atom name '" << name << "' to '" << name.getField(0) << "'." << std::endl;
			}
			else
			{
				getFileStream() << name.trim() << " ";
			}
		}
		else
		{
			getFileStream() << "- ";
		}

		getFileStream() << atom.getElement().getSymbol().trim() << " ";
		if ((atom.getTypeName() == "?") || (atom.getTypeName() == ""))
		{
			getFileStream() << "**";
		}
		else
		{
			getFileStream() << atom.getTypeName();
		}
		getFileStream() << " - ";
		getFileStream() << atom.getCharge() << " ";
		getFileStream() << atom.getPosition().x << " ";
		getFileStream() << atom.getPosition().y << " ";
		getFileStream() << atom.getPosition().z << " ";

		Size number_of_bonds = 0;
		String bond_string(" ");
		const Atom* bond_partner;

		// count the valid bonds (bonds to atoms inside the system)
		for (Position i = 0; i < atom.countBonds(); ++i)
		{
			const Bond* bond = atom.getBond(i);
			bond_partner = bond->getPartner(atom);

			Size index = bond_partner->getProperty("__HINFILE_INDEX").getUnsignedInt();
			if (index != 0)
			{
				number_of_bonds++;

				bond_string += String(index);

				switch (bond->getOrder())
				{
					case Bond::ORDER__DOUBLE:   bond_string += " d "; break;
					case Bond::ORDER__TRIPLE:   bond_string += " t "; break;
					case Bond::ORDER__AROMATIC: bond_string += " a "; break;
					// default is single bond!
					default:										bond_string += " s ";
				}
			}
		}

		// write the bonds
		getFileStream() << number_of_bonds << bond_string << std::endl;

		// HyperChem uses A/ps, as does BALL. So, no conversion is needed.
		getFileStream() << "vel " << number + 1 - atom_offset << " "
								 << atom.getVelocity().x << " "
								 << atom.getVelocity().y << " "
								 << atom.getVelocity().z << std::endl;
	}

	bool HINFile::write(const Molecule& molecule)
	{
		System S;
		S.insert(*(Molecule*)molecule.create(true));
		return write(S);
	}

	bool HINFile::write(const System& system)
	{
		if (!isOpen() || getOpenMode() != std::ios::out)
		{
			throw(File::CannotWrite(__FILE__, __LINE__, name_));
		}

		// the atom_vector contains the atoms in the order of
		// the atom iterator
		vector<const Atom*> atom_vector;

		// create a vector containing pointers to the atoms
		AtomConstIterator atom_it;
		for (atom_it = system.beginAtom(); +atom_it; ++atom_it)
		{
			atom_vector.push_back(&(*atom_it));
			(const_cast<Atom&>(*atom_it)).setProperty("__HINFILE_INDEX", (unsigned int)atom_vector.size());
		}

		// the index_vector contains the index of the connected component
		// (HyperChem molecule) it is in and initialize it to zero
		vector<Index>  index_vector(atom_vector.size(), -1);

		typedef list<Size> Component;
		typedef	vector<Component> ComponentVector;

		// now calculate all connected components in the graph
		// formed by atoms and bonds of the system
		// each of these connected components represents a molecule
		// for the new HyperChem file

		// index of the current connected component
		Size current_index = 0;

		// the index of the atom to start a new component
		Size start_index = 0;

		// search components until all atoms have been considered
		while (start_index < atom_vector.size())
		{
			while ((start_index < atom_vector.size()) && (index_vector[start_index] >= 0))
			{
				start_index++;
			}

			if (start_index < atom_vector.size())
			{
				// create a stack containing all atoms to be axamined for this component
				std::stack<Size> atom_stack;

				// our start atom is the first to be considered and is marked, too
				atom_stack.push(start_index);
				index_vector[start_index] = (Index)current_index;

				// never examine this atom again as start atom
				start_index++;

				// calculate the connected component
				while (!atom_stack.empty())
				{
					// check all bonds of this atom
					// and remove it from the stack
					const Atom& current_atom = *atom_vector[atom_stack.top()];
					atom_stack.pop();

					Atom::BondConstIterator bond_it = current_atom.beginBond();
					for (; +bond_it; ++bond_it)
					{
						// add the atom if it is not marked yet
						// ignore all bonds to atoms outside the system 
						// (these atoms have not been marked, so getProperty will return 0)
						Size atom_index = bond_it->getPartner(current_atom)
																->getProperty("__HINFILE_INDEX").getUnsignedInt();
						if ((atom_index != 0) && (index_vector[atom_index - 1] == -1))
						{
							// remember this atom in the stack
							// and mark it in the index_vector
							atom_stack.push(atom_index - 1);
							index_vector[atom_index - 1] = (Index)current_index;
						}
					}
				}

				// done with this component, increase the component counter
				current_index++;
			}
		}

		// create an empty vector for the connected components
		// 
		ComponentVector components(current_index);

		// now extract lists of all connected component 
		// in the order of the atom_vector to keep the
		// order of residues
		for (Size i = 0; i < atom_vector.size(); i++)
		{
			// remember the index of the atom
			components[index_vector[i]].push_back(i);

			// and set the atom's HINFILE_INDEX properly
			// (i.e. to the index in the right connected component
			(const_cast<Atom*>(atom_vector[i]))->setProperty("__HINFILE_INDEX",
																	(unsigned int)components[index_vector[i]].size());
		}

		// write some default header
		getFileStream() << "; HyperChem file created by BALL" << std::endl;
		getFileStream() << ";" << std::endl;
		getFileStream() << "forcefield AMBER" << std::endl;

		// ?????: 
		// insert system temperature here
		getFileStream() << "sys 0" << std::endl;

		// ?????:
		// insert the periodic box size (if any)

		Size atom_count = 0;
		Size atom_offset = 0;

		// for each connected component: write a molecule
		for (Size j = 0; j < current_index; j++)
		{
			// renumber all atoms relative to the first atom of
			// each connected component to obtain a numbering starting
			// with 1 for each molecule
			atom_offset = atom_count;

			// try to find a name for the molecule
			const Molecule* mol = atom_vector[components[j].front()]->getMolecule();
			String name = "-";
			if (mol != 0)
			{
				name = mol->getName();
				name.trim();
			}

			// Write the molecule identifier	
			// For recent versions of HyperChem, the name has to be enclosed in double quotes.
			if (name != "")
			{
				// Make sure the name does not contain double quotes.
				while (name.substitute("\"", "") != String::EndPos) {};
				getFileStream() << "mol " << j + 1 << " \"" << name << "\"" << std::endl;
			}
			else
			{
				getFileStream() << "mol " << j + 1 << std::endl;
			}
			
			// now iterate over all atoms and write them
			const Residue* current_residue = 0;

			// the residues start at zero in each molecule as do the atoms
			Size res_count = 0;			
			Component::const_iterator comp_it = components[j].begin();
			for (; comp_it != components[j].end(); comp_it++)
			{
				// counter for the residues
				const Atom* this_atom = atom_vector[*comp_it];
					
				const Residue* this_residue = this_atom->Composite::getAncestor(RTTI::getDefault<Residue>());
				if (this_residue != current_residue)
				{
					if (current_residue != 0)
					{
						// write and "endres" tag
						getFileStream() << "endres " << res_count << std::endl;
					}

					current_residue = this_residue;

					if (this_residue != 0)
					{
						res_count++;

						// write a new residue tag
						name = this_residue->getName();
						name.trim();
						if (name == "")
						{
							name = "-";
						}
						
						getFileStream() << "res " << res_count << " ";
						getFileStream() << name << " ";
						
						name = this_residue->getID();
						name.trim();
						if (name == "")
						{
							// If the ID is not set, it defaults to the
							// current residue number.
							name = String(res_count - 1);
						}

						getFileStream() << name << " - ";
						
						// write the chain name
						const Chain*	chain = this_residue->getChain();
						if ((chain != 0)) 
						{
							name = chain->getName();
							name.trim();
							
							if (name == "")
							{
								name = "-";
							}
						} 
						else 
						{
							name = "-";
						}

						getFileStream() << "-" << std::endl;
					}
				}

				// write the atom
				writeAtom_(*this_atom, atom_count++, atom_offset);
			}

			// if the last atom was inside a residue, write the 
			// endres tag
			if (current_residue != 0)
			{
				// write and "endres" tag
				getFileStream() << "endres " << res_count << std::endl;
			}

			// write endmol keyword
			getFileStream() << "endmol " << j + 1 << std::endl;
		}

		// clear the atom properties
		for (atom_it = system.beginAtom(); +atom_it; ++atom_it)
		{
			(const_cast<Atom&>(*atom_it)).clearProperty("__HINFILE_INDEX");
		}

		return true;
	}

	bool HINFile::read(System& system)
	{
		return GenericMolFile::read(system);
	}
	
	Molecule* HINFile::read()
	{
		if (!isValid())
		{
			Log.error() << "trying to read form invalid HINFile '" << getName() << "'" << std::endl;
			return 0;
		}
		
		// we define some states for our simple parser machine
		// legal transitions are:
		// START->IN_MOLECULE, IN_MOLECULE->IN_RESIDUE
		// IN_RESIDUE->IN_MOLECULE, IN_MOLECULE->START
		enum State 
		{
			START = 0,
			IN_MOLECULE,
			IN_RESIDUE
		};
		
		// define a macro to print an error message for the file (only once!)
#		define ERROR(msg)\
				throw Exception::ParseError(__FILE__, __LINE__, getLine(), String("while reading line " + String(getLineNumber()) + " of '" + getName() + "': " + (msg)));
			
		// the current state - we don't want to insert an atom if
		// we don't have created the molecule/residue to insert it into!
		State	state = START;

		// 
		// All <mol>s that contain <res>idues are inserted
		// as single chains into this protein
    // 
		Residue*	residue = 0;
		Molecule*	molecule = 0;	
		Fragment*	fragment = 0;
		Protein*  protein = 0;
		Chain*    chain = 0;

		// initial size: 100 atoms, all set to NULL pointer, 100 bonds
		std::vector<Atom*> atom_vector(100);
		Position last_atom = 0;
		static std::vector<struct HINFileBondStruct> bond_vector;
		bond_vector.clear();

		String tag;

		try
		{
			while (readLine() && (getLine() != "") && good()) 
			{
				// ignore comment lines
				if (getLine()[0] == ';' || getLine() == "") 
				{
					continue;
				}

				// determine the hyperchem tag: always the first word in a line
				tag = getLine().getField(0);

				if (tag == "atom")
				{
					if (state == IN_RESIDUE || state == IN_MOLECULE) 
					{
						Size number_of_fields = getLine().countFields();
						if (number_of_fields < 11) 
						{
							ERROR(String("illegal <atom> line: at least 10 arguments are required for <atom>!"))
						}

						Atom*	atom;
						if (state == IN_RESIDUE) 
						{
							PDBAtom* prot_atom = new PDBAtom;
							atom = RTTI::castTo<Atom>(*prot_atom);
							residue->insert(*prot_atom);

							// check the atom flags, whether this is a PDB HETATM: 
							// any HETATM sets its residue to NON_STANDARD 
							// (compare FORMAT/PDBFile:readRecordHETATM)
							if (getLine().getField(5).has('h')) 
							{
								residue->clearProperty(Residue::PROPERTY__AMINO_ACID);
								residue->setProperty(Residue::PROPERTY__NON_STANDARD);
							} 
							else 
							{
								residue->setProperty(Residue::PROPERTY__AMINO_ACID);
								residue->clearProperty(Residue::PROPERTY__NON_STANDARD);
							}
						} 
						else 
						{
							atom = new Atom;
							if (molecule == 0) 
							{
								fragment->insert(*atom);
							} 
							else 
							{
								molecule->insert(*atom);
							}
						}

						atom->setName(getLine().getField(2));

						if (PTE[getLine().getField(3)] == Element::UNKNOWN)
						{
							// Treat "lone pair atoms" (Lp) written by Amber -- we just keep them
							// for now and remove them later.
							if (getLine().getField(3) != "Lp")
							{
								ERROR(String("unknown element: ") + getLine().getField(3))
							}
						}
						// Set the element and the atom radius (vdW radius).
						atom->setElement(PTE[getLine().getField(3)]);
						atom->setRadius(PTE[getLine().getField(3)].getVanDerWaalsRadius());

						if (getLine().getField(4) == "**")
						{
							atom->setTypeName("?");
						} 
						else	
						{
							atom->setTypeName(getLine().getField(4));
						}

						try 
						{
							atom->setCharge(getLine().getField(6).toFloat());
						}
						catch (Exception::InvalidFormat&)
						{
							ERROR(String("illegal charge ") + getLine().getField(6))
						}

						try 
						{
							atom->setPosition(Vector3(getLine().getField(7).toFloat(), 
																				getLine().getField(8).toFloat(), 
																				getLine().getField(9).toFloat()));
						}
						catch (Exception::InvalidFormat&)
						{
							ERROR(String("illegal position (") + getLine().getField(7) + " / " + getLine().getField(8) + " / " + getLine().getField(9) + ")")
						}

						Size number_of_atom_bonds;
						try 
						{
							number_of_atom_bonds = ((Size)getLine().getField(10).toInt());
						}
						catch (Exception::InvalidFormat&)
						{
							ERROR(String("illegal number of bonds: " + getLine().getField(10)))
						}

						Position atom_number;
						try 
						{
							atom_number = (Position)getLine().getField(1).toInt() - 1;
						}
						catch (Exception::InvalidFormat&)
						{
							ERROR(String("illegal atom number: ") + getLine().getField(1))
						}

						// Store the atom pointer in the atom_vector - we need it later to create the bonds!
						if (atom_number >= atom_vector.size())
						{
							atom_vector.resize(atom_vector.size() * 2);
						}
						atom_vector[atom_number] = atom;
						if ((atom_number <= last_atom) && (last_atom > 0))
						{
							ERROR(String("unordered atom indices: ") + getLine())
						}
						last_atom = atom_number;

						// now iterate over all bonds and insert them into the bond_vector 
						// this table will be processed afterwards to create the bonds, as most of
						// the bound atoms are not yet created by now
						if (number_of_atom_bonds > 0)
						{
							// check whether the total number of fields is consistent
							// with the number of bonds
							if (number_of_fields != (11 + 2 * number_of_atom_bonds))
							{
								// write an error message!
								if (number_of_fields < (11 + 2 * number_of_atom_bonds))
								{
									ERROR(String("too few fields in atom line for ") + String(number_of_atom_bonds) + " bonds")
								}
								else 
								{
									ERROR(String("too many fields in atom line for ") + String(number_of_atom_bonds) + " bonds")
								}
							}
							else 
							{
								for (Position i = 0 ; i < number_of_atom_bonds; i++) 
								{ 
									struct HINFileBondStruct bond;
									bond.atom1 = atom_number;
									try 
									{
										bond.atom2 = (Index)getLine().getField(11 + 2 * (Index)i).toInt() - 1;
									}
									catch (Exception::InvalidFormat&)
									{
										ERROR(String("illegal atom number for bond ") + String(i) + ": " + getLine().getField(11 + 2 * (Index)i))
									}
									Bond::Order order = Bond::ORDER__UNKNOWN;
									String type_field = getLine().getField(12 + 2 * (Index)i);

									if (type_field.size() == 1)
									{
										switch (type_field[0]) 
										{
											case 's':	order = Bond::ORDER__SINGLE;		break;
											case 'd':	order = Bond::ORDER__DOUBLE;		break;
											case 't': order = Bond::ORDER__TRIPLE;		break;
											case 'a': order = Bond::ORDER__AROMATIC;	break;
										}
									}
									bond.order = order;
									bond_vector.push_back(bond);
								}
							}
						}
					}	
					else 
					{
						ERROR(String("<atom> tag may appear only inside a <mol> or <res>!"))
					}
					continue;
				}


				if (tag == "vel")
				{
					// read the velocity of an atom
					// since HyperChem uses the same units for velocities 
					// (resp. A/ps) we don't need a conversion
					Vector3 velocity;

					try
					{
						velocity.x = getLine().getField(2).toFloat();
						velocity.y = getLine().getField(3).toFloat();
						velocity.z = getLine().getField(4).toFloat();
					}
					catch (Exception::InvalidFormat&)
					{
						ERROR(String("illegal velocity (") 
												 + getLine().getField(2) + " / "
												 + getLine().getField(3) + " / " 
												 + getLine().getField(4) + ")")
					}

					// check whether the atom exists
					Position atom_number;
					try
					{
						atom_number = (Position)getLine().getField(1).toInt() - 1;
					}
					catch (Exception::InvalidFormat&)
					{
						ERROR(String("illegal atom number ") + getLine().getField(1))
					}

					if (atom_number > last_atom)
					{
						ERROR(String("cannot assign velocity for atom ") + String(atom_number) + ": atom not defined!")
					}

					if (atom_vector[atom_number] != 0)
					{
						atom_vector[atom_number]->setVelocity(velocity);
					}
					else
					{
						ERROR(String("cannot assign velocity for atom ") + String(atom_number) + ": atom not defined!")
					}

					continue;
				}


				if (tag == "res")
				{
					// remember where we are.
					if (state != IN_MOLECULE)
					{
						ERROR(String("<res> tag must be inside a <mol>/<endmol>"))
					}

					state = IN_RESIDUE;

					// create a protein if it doesn't exist already
					if (protein == 0)
					{
						protein = new Protein;
					}

					// check whether we already have a chain to insert into
					if (chain == 0)
					{
						protein->insert(*(chain = new Chain));
					}

					// create a new residue and insert it into the chain
					residue = new Residue;
					chain->insert(*residue);

					// set the residue's name
					if (getLine().getField(2) != "-")
					{
						residue->setName(getLine().getField(2));
					}

					// set the residue's PDB ID
					if (getLine().getField(3) != "-")
					{
						residue->setID(getLine().getField(3));
					}

					// set the chain name 
					if (getLine().getField(5) != "-")
					{
						chain->setName(getLine().getField(5));
					}
					
					// create a fragment to insert the "loose" atoms into
					if (fragment == 0)
					{
						fragment = new Fragment;
						chain->AtomContainer::insert(*fragment);
					}

					// now check for a molecule, that might already exist
					// and move its atoms into a fragment (to be inserted into the chain)
					if (molecule == 0)
					{
						continue;
					}

					AtomIterator atom_it = molecule->beginAtom();
					vector<Atom*>	tmp_atoms;
					for (; +atom_it; ++atom_it)
					{
						tmp_atoms.push_back(&*atom_it);
					}
					for (Size i = 0; i < tmp_atoms.size(); ++i)
					{
						fragment->insert(*tmp_atoms[i]);
					}
					
					// delete the (now empty!) molecule and clear the pointer
					delete molecule;
					molecule = 0;
					continue;
				}


				if (tag == "endres")
				{
					if (state != IN_RESIDUE) 
					{
						ERROR(String("<endres> without <res>!"))
					}
					
					state = IN_MOLECULE;

					// reset the residue pointer to zero
					residue = 0;
					
					continue;
				}


				if (tag == "mol")
				{
					if (state != START) 
					{
						ERROR(String("<mol> inside <mol> or <res> definition!"))
					}

					state = IN_MOLECULE;

					// create a new molecule and insert it into the system.
					// We do not yet know, whether this contains residues.
					// If it does, we have to convert it to a protein afterwards.
					molecule = new Molecule;

					if (getLine().countFields() > 2)
					{
						// Determine the thrid field (the molecule name).
						// The name may (latest versions) be enclosed by double quotes.
						std::vector<String> fields;
						getLine().splitQuoted(fields, String::CHARACTER_CLASS__WHITESPACE, "\"");

						// Make sure that the name field is not empty as
						// splitQuoted silently drops empty fields
						if(fields.size() > 2)
						{
							String name(fields[2]);

							if ((name != "") && (name != "-"))
							{
								// Remove leading/trailing blanks from the name.
								name.trim();

								// For newer versions of HyperChem, the name has to be
								// enclosed in double quotes.
								if (name[0] == '"')
								{
									name.erase(0, 1);
								}
								if (name[name.size() - 1] == '"')
								{
									name.erase(name.size() - 1, 1);
								}
								molecule->setName(name);
							}
						}
					}

					continue;
				}

				if (tag == "endmol")
				{
					if (state != IN_MOLECULE)
					{
						ERROR(String("missing <mol> or <endres> tag!"))
					}

					state = START;

					if (fragment != 0)
					{
						if (fragment->countAtoms() == 0)
						{
							delete fragment;
						}
					}
					fragment = 0;
					if (chain != 0)
					{
						if (chain->countAtoms() == 0)
						{
							delete chain;
						}
					}
					chain = 0;


					// Now, build all bonds
					for (Size i = 0; i < bond_vector.size(); i++)
					{
						// check whether both atoms exist
						if (   bond_vector[i].atom1 > last_atom
								|| bond_vector[i].atom2 > last_atom)
						{
							// complain if one of the atoms does not exist
							ERROR(String("HINFile: cannot create bond from atom ") + String(bond_vector[i].atom1 + 1)
										+ " to atom " + String(bond_vector[i].atom2 + 1) + " of molecule "
										+ getLine().getField(1) + " - non-existing atom!")
						}
						else
						{
							// everything all right, create the bond
							Bond* b = atom_vector[bond_vector[i].atom1]->createBond(*atom_vector[bond_vector[i].atom2]);
							b->setOrder(bond_vector[i].order);
							
							// Fix up the disulphide bridges in proteins.
							// Make sure we have
							//   -- a protein
							//   -- two sulfurs
							//   -- connecting different residues
							//   -- these residues are not hetero residues.
							PDBAtom* a1 = dynamic_cast<PDBAtom*>(const_cast<Atom*>(b->getFirstAtom()));
							PDBAtom* a2 = dynamic_cast<PDBAtom*>(const_cast<Atom*>(b->getSecondAtom()));
							if ((a1 != 0) && (a2 != 0)
									&& (a1->getElement() == PTE[Element::S]) && (a2->getElement() == PTE[Element::S])
									&& (a1->getResidue() != a2->getResidue())
									&& (a1->getResidue() != 0) && (a2->getResidue() != 0)
									&& (a1->getResidue()->hasProperty(Residue::PROPERTY__AMINO_ACID)) 
									&& (a2->getResidue()->hasProperty(Residue::PROPERTY__AMINO_ACID)))
							{
								a1->getResidue()->setProperty(Residue::PROPERTY__HAS_SSBOND);
								a2->getResidue()->setProperty(Residue::PROPERTY__HAS_SSBOND);
							}
						}
					}
					bond_vector.clear();
					atom_vector.clear();
					last_atom = 0;

					throw(Exception::IndexOverflow(__FILE__, __LINE__));
				}


				if (tag == "sys")
				{
					// set the system's temperature
					try
					{
						temperature_ = getLine().getField(1).toFloat();
					}
					catch (Exception::InvalidFormat&)
					{
						ERROR(String("illegal temperature ") + getLine().getField(1))
					}

					continue;
				}


				if (tag == "box")
				{
					// retrieve the periodic boundary
					// we assume that the box is centered about the origin
					// of the coordinate system
					// The manual says the parameters are the dimensions of the box,
					// so we have to divide by two.
					try
					{
						box_.a.x = - getLine().getField(1).toFloat() / 2.0;
						box_.a.y = - getLine().getField(2).toFloat() / 2.0;
						box_.a.z = - getLine().getField(3).toFloat() / 2.0;
					}
					catch (Exception::InvalidFormat&)
					{
						ERROR(String("illegal box position (")
												 + getLine().getField(1)  + " / "
												 + getLine().getField(2)  + " / "
												 + getLine().getField(3)  + " )")
					}

					box_.b.x = - box_.a.x;
					box_.b.y = - box_.a.y;
					box_.b.z = - box_.a.z;
					continue;
				}


				// ignore the irrelevant fields
				if (tag == "forcefield" ||
						tag == "user1color" ||
						tag == "user2color" ||
						tag == "user3color" ||
						tag == "user4color" ||
						tag == "view"				||
						tag == "seed"				||
						tag == "mass"				||
						tag == "basisset"		||
						tag == "selection"	||
						tag == "endselection"		 ||
						tag == "selectrestraint" ||
						tag == "selectatom"			 ||
						tag == "formalcharge")
				{
					continue;
				}

				// if the tag was not recognized: complain about it
				Log.warn() << "HINFile: unknown tag " << tag << " ignored." << std::endl;
			}
		}
		catch (Exception::ParseError& e)
		{
			// Delete all stray atoms. The order is important:
			// since fragment and residue could be contained in 
			// chain, molecule, etc., they have to be deleted first!
			delete fragment;
			delete residue;
			delete chain;
			delete molecule;
			delete protein;
			throw e;
		}
		catch (Exception::IndexOverflow&)
		{
		}

		// if we read a protein, return it as a molecule anyway
		if (molecule == 0)
		{
			molecule = protein;
			protein = 0;
		}

		// Remove the lone pairs from old AMBER HC-Files 
		if (molecule != 0)
		{
			// a list to hold the lone pairs (for deletion)
			list<Atom*> del_list;

			// Iterate over all atoms
			AtomIterator it = molecule->beginAtom();
			for (; +it; ++it)
			{
				if (it->getElement().getSymbol() == "Lp")
				{
					// store lone pair in the del_list
					del_list.push_back(&*it);

					// sum the lone pair charge into the
					// heavy atom it is bound to
					if (it->countBonds() > 0)
					{
						float charge = it->getCharge() / (float)it->countBonds();
						for (Atom::BondIterator bond_it = it->beginBond(); +bond_it; ++bond_it)
						{
							Atom* partner = bond_it->getPartner(*it);
							if (partner != 0)
							{
								partner->setCharge(partner->getCharge() + charge);
							}
						}
					}

					// remove all bonds to the lone pair
					it->destroyBonds();
				}
			}

			// remove the lone pairs
			list<Atom*>::iterator list_it = del_list.begin();
			for (; list_it != del_list.end(); ++list_it)
			{
				if ((*list_it)->isAutoDeletable())
				{
					// delete dynamically created objects
					delete *list_it;
				}
				else
				{
					// destroy static atoms
					(*list_it)->destroy();
				}
			}
		}

		// return the resulting molecule
		return molecule;
	}

	void HINFile::initRead_()
	{
		// reset some private members
		box_.a.set(0.0);
		box_.b.set(0.0);
		temperature_ = 0.0;
	}

	bool HINFile::hasPeriodicBoundary() const
	{
		return (box_.a != box_.b);
	}

	SimpleBox3 HINFile::getPeriodicBoundary() const
	{
		return box_;
	}

	float HINFile::getTemperature() const
	{
		return temperature_;
	}

} // namespace BALL
