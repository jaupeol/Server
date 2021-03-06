/*	EQEMu: Everquest Server Emulator
	Copyright (C) 2001-2003 EQEMu Development Team (http://eqemulator.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; version 2 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY except by those people which sell it, which
	are required to give you total support for your newly bought product;
	without even the implied warranty of MERCHANTABILITY or FITNESS FOR
	A PARTICULAR PURPOSE. See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "../common/debug.h"
#include "../common/logsys.h"
#include "../common/string_util.h"
#include "quest_parser_collection.h"
#include "worldserver.h"
#include "zonedb.h"

extern WorldServer worldserver;

// @merth: this needs to be touched up
uint32 Client::NukeItem(uint32 itemnum, uint8 where_to_check) {
	if (itemnum == 0)
		return 0;
	uint32 x = 0;
	ItemInst *cur = nullptr;

	int i;
	if(where_to_check & invWhereWorn) {
		for (i = EmuConstants::EQUIPMENT_BEGIN; i <= EmuConstants::EQUIPMENT_END; i++) {
			if (GetItemIDAt(i) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(i) != INVALID_ID)) {
				cur = m_inv.GetItem(i);
				if(cur && cur->GetItem()->Stackable) {
					x += cur->GetCharges();
				} else {
					x++;
				}

				DeleteItemInInventory(i, 0, true);
			}
		}

		if (GetItemIDAt(MainPowerSource) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(MainPowerSource) != INVALID_ID)) {
			cur = m_inv.GetItem(MainPowerSource);
			if(cur && cur->GetItem()->Stackable) {
				x += cur->GetCharges();
			} else {
				x++;
			}

			if (GetClientVersion() >= EQClientSoF)
				DeleteItemInInventory(MainPowerSource, 0, true);
			else
				DeleteItemInInventory(MainPowerSource, 0, false);	// Prevents Titanium crash
		}
	}

	if(where_to_check & invWhereCursor) {
		if (GetItemIDAt(MainCursor) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(MainCursor) != INVALID_ID)) {
			cur = m_inv.GetItem(MainCursor);
			if(cur && cur->GetItem()->Stackable) {
				x += cur->GetCharges();
			} else {
				x++;
			}

			DeleteItemInInventory(MainCursor, 0, true);
		}

		for (i = EmuConstants::CURSOR_BAG_BEGIN; i <= EmuConstants::CURSOR_BAG_END; i++) {
			if (GetItemIDAt(i) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(i) != INVALID_ID)) {
				cur = m_inv.GetItem(i);
				if(cur && cur->GetItem()->Stackable) {
					x += cur->GetCharges();
				} else {
					x++;
				}

				DeleteItemInInventory(i, 0, true);
			}
		}
	}

	if(where_to_check & invWherePersonal) {
		for (i = EmuConstants::GENERAL_BEGIN; i <= EmuConstants::GENERAL_END; i++) {
			if (GetItemIDAt(i) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(i) != INVALID_ID)) {
				cur = m_inv.GetItem(i);
				if(cur && cur->GetItem()->Stackable) {
					x += cur->GetCharges();
				} else {
					x++;
				}

				DeleteItemInInventory(i, 0, true);
			}
		}

		for (i = EmuConstants::GENERAL_BAGS_BEGIN; i <= EmuConstants::GENERAL_BAGS_END; i++) {
			if (GetItemIDAt(i) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(i) != INVALID_ID)) {
				cur = m_inv.GetItem(i);
				if(cur && cur->GetItem()->Stackable) {
					x += cur->GetCharges();
				} else {
					x++;
				}

				DeleteItemInInventory(i, 0, true);
			}
		}
	}

	if(where_to_check & invWhereBank) {
		for (i = EmuConstants::BANK_BEGIN; i <= EmuConstants::BANK_END; i++) {
			if (GetItemIDAt(i) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(i) != INVALID_ID)) {
				cur = m_inv.GetItem(i);
				if(cur && cur->GetItem()->Stackable) {
					x += cur->GetCharges();
				} else {
					x++;
				}

				DeleteItemInInventory(i, 0, true);
			}
		}

		for (i = EmuConstants::BANK_BAGS_BEGIN; i <= EmuConstants::BANK_BAGS_END; i++) {
			if (GetItemIDAt(i) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(i) != INVALID_ID)) {
				cur = m_inv.GetItem(i);
				if(cur && cur->GetItem()->Stackable) {
					x += cur->GetCharges();
				} else {
					x++;
				}

				DeleteItemInInventory(i, 0, true);
			}
		}
	}

	if(where_to_check & invWhereSharedBank) {
		for (i = EmuConstants::SHARED_BANK_BEGIN; i <= EmuConstants::SHARED_BANK_END; i++) {
			if (GetItemIDAt(i) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(i) != INVALID_ID)) {
				cur = m_inv.GetItem(i);
				if(cur && cur->GetItem()->Stackable) {
					x += cur->GetCharges();
				} else {
					x++;
				}

				DeleteItemInInventory(i, 0, true);
			}
		}

		for (i = EmuConstants::SHARED_BANK_BAGS_BEGIN; i <= EmuConstants::SHARED_BANK_BAGS_END; i++) {
			if (GetItemIDAt(i) == itemnum || (itemnum == 0xFFFE && GetItemIDAt(i) != INVALID_ID)) {
				cur = m_inv.GetItem(i);
				if(cur && cur->GetItem()->Stackable) {
					x += cur->GetCharges();
				} else {
					x++;
				}

				DeleteItemInInventory(i, 0, true);
			}
		}
	}

	return x;
}


bool Client::CheckLoreConflict(const Item_Struct* item) {
	if (!item)
		return false;
	if (!(item->LoreFlag))
		return false;

	if (item->LoreGroup == -1)	// Standard lore items; look everywhere except the shared bank, return the result
		return (m_inv.HasItem(item->ID, 0, ~invWhereSharedBank) != INVALID_INDEX);

	//If the item has a lore group, we check for other items with the same group and return the result
	return (m_inv.HasItemByLoreGroup(item->LoreGroup, ~invWhereSharedBank) != INVALID_INDEX);
}

bool Client::SummonItem(uint32 item_id, int16 charges, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5, uint32 aug6, bool attuned, uint16 to_slot, uint32 ornament_icon, uint32 ornament_idfile, uint32 ornament_hero_model) {
	this->EVENT_ITEM_ScriptStopReturn();

	// TODO: update calling methods and script apis to handle a failure return

	const Item_Struct* item = database.GetItem(item_id);

	// make sure the item exists
	if(item == nullptr) {
		Message(13, "Item %u does not exist.", item_id);
		mlog(INVENTORY__ERROR, "Player %s on account %s attempted to create an item with an invalid id.\n(Item: %u, Aug1: %u, Aug2: %u, Aug3: %u, Aug4: %u, Aug5: %u, Aug6: %u)\n",
			GetName(), account_name, item_id, aug1, aug2, aug3, aug4, aug5, aug6);

		return false;
	}
	// check that there is not a lore conflict between base item and existing inventory
	else if(CheckLoreConflict(item)) {
		// DuplicateLoreMessage(item_id);
		Message(13, "You already have a lore %s (%i) in your inventory.", item->Name, item_id);

		return false;
	}
	// check to make sure we are augmenting an augmentable item
	else if (((item->ItemClass != ItemClassCommon) || (item->AugType > 0)) && (aug1 | aug2 | aug3 | aug4 | aug5 | aug6)) {
		Message(13, "You can not augment an augment or a non-common class item.");
		mlog(INVENTORY__ERROR, "Player %s on account %s attempted to augment an augment or a non-common class item.\n(Item: %u, Aug1: %u, Aug2: %u, Aug3: %u, Aug4: %u, Aug5: %u, Aug5: %u)\n",
			GetName(), account_name, item->ID, aug1, aug2, aug3, aug4, aug5, aug6);

		return false;
	}

	// This code is ready to implement once the item load code is changed to process the 'minstatus' field.
	// Checking #iteminfo in-game verfies that item->MinStatus is set to '0' regardless of field value.
	// An optional sql script will also need to be added, once this goes live, to allow changing of the min status.

	// check to make sure we are a GM if the item is GM-only
	/*
	else if(item->MinStatus && ((this->Admin() < item->MinStatus) || (this->Admin() < RuleI(GM, MinStatusToSummonItem)))) {
		Message(13, "You are not a GM or do not have the status to summon this item.");
		mlog(INVENTORY__ERROR, "Player %s on account %s attempted to create a GM-only item with a status of %i.\n(Item: %u, Aug1: %u, Aug2: %u, Aug3: %u, Aug4: %u, Aug5: %u, Aug6: %u, MinStatus: %u)\n",
			GetName(), account_name, this->Admin(), item->ID, aug1, aug2, aug3, aug4, aug5, aug6, item->MinStatus);

		return false;
	}
	*/

	uint32 augments[EmuConstants::ITEM_COMMON_SIZE] = { aug1, aug2, aug3, aug4, aug5, aug6 };

	uint32 classes	= item->Classes;
	uint32 races	= item->Races;
	uint32 slots	= item->Slots;

	bool enforcewear	= RuleB(Inventory, EnforceAugmentWear);
	bool enforcerestr	= RuleB(Inventory, EnforceAugmentRestriction);
	bool enforceusable	= RuleB(Inventory, EnforceAugmentUsability);
	
	for (int iter = AUG_BEGIN; iter < EmuConstants::ITEM_COMMON_SIZE; ++iter) {
		const Item_Struct* augtest = database.GetItem(augments[iter]);

		if(augtest == nullptr) {
			if(augments[iter]) {
				Message(13, "Augment %u (Aug%i) does not exist.", augments[iter], iter + 1);
				mlog(INVENTORY__ERROR, "Player %s on account %s attempted to create an augment (Aug%i) with an invalid id.\n(Item: %u, Aug1: %u, Aug2: %u, Aug3: %u, Aug4: %u, Aug5: %u, Aug6: %u)\n",
					GetName(), account_name, (iter + 1), item->ID, aug1, aug2, aug3, aug4, aug5, aug6);

				return false;
			}
		}
		else {
			// check that there is not a lore conflict between augment and existing inventory
			if(CheckLoreConflict(augtest)) {
				// DuplicateLoreMessage(augtest->ID);
				Message(13, "You already have a lore %s (%u) in your inventory.", augtest->Name, augtest->ID);
				
				return false;
			}
			// check that augment is an actual augment
			else if(augtest->AugType == 0) {
				Message(13, "%s (%u) (Aug%i) is not an actual augment.", augtest->Name, augtest->ID, iter + 1);
				mlog(INVENTORY__ERROR, "Player %s on account %s attempted to use a non-augment item (Aug%i) as an augment.\n(Item: %u, Aug1: %u, Aug2: %u, Aug3: %u, Aug4: %u, Aug5: %u, Aug6: %u)\n",
					GetName(), account_name, item->ID, (iter + 1), aug1, aug2, aug3, aug4, aug5, aug6);
				
				return false;
			}

			// Same as GM check above

			// check to make sure we are a GM if the augment is GM-only
			/*
			else if(augtest->MinStatus && ((this->Admin() < augtest->MinStatus) || (this->Admin() < RuleI(GM, MinStatusToSummonItem)))) {
				Message(13, "You are not a GM or do not have the status to summon this augment.");
				mlog(INVENTORY__ERROR, "Player %s on account %s attempted to create a GM-only augment (Aug%i) with a status of %i.\n(Item: %u, Aug1: %u, Aug2: %u, Aug3: %u, Aug4: %u, Aug5: %u, MinStatus: %u)\n",
					GetName(), account_name, (iter + 1), this->Admin(), item->ID, aug1, aug2, aug3, aug4, aug5, aug6, item->MinStatus);

				return false;
			}
			*/

			// check for augment type allowance
			if(enforcewear) {
				if((item->AugSlotType[iter] == AugTypeNone) || !(((uint32)1 << (item->AugSlotType[iter] - 1)) & augtest->AugType)) {
					Message(13, "Augment %u (Aug%i) is not acceptable wear on Item %u.", augments[iter], iter + 1, item->ID);
					mlog(INVENTORY__ERROR, "Player %s on account %s attempted to augment an item with an unacceptable augment type (Aug%i).\n(Item: %u, Aug1: %u, Aug2: %u, Aug3: %u, Aug4: %u, Aug5: %u, Aug6: %u)\n",
						GetName(), account_name, (iter + 1), item->ID, aug1, aug2, aug3, aug4, aug5, aug6);

					return false;
				}

				if(item->AugSlotVisible[iter] == 0) {
					Message(13, "Item %u has not evolved enough to accept Augment %u (Aug%i).", item->ID, augments[iter], iter + 1);
					mlog(INVENTORY__ERROR, "Player %s on account %s attempted to augment an unevolved item with augment type (Aug%i).\n(Item: %u, Aug1: %u, Aug2: %u, Aug3: %u, Aug4: %u, Aug5: %u, Aug6: %u)\n",
						GetName(), account_name, (iter + 1), item->ID, aug1, aug2, aug3, aug4, aug5, aug6);

					return false;
				}
			}

			// check for augment to item restriction
			if(enforcerestr) {
				bool restrictfail = false;
				uint8 it = item->ItemType;

				switch(augtest->AugRestrict) {
				case AugRestrAny:
					break;
				case AugRestrArmor:
					switch(it) {
					case ItemTypeArmor:
						break;
					default:
						restrictfail = true;
						break;
					}
					break;
				case AugRestrWeapons:
					switch(it) {
					case ItemType1HSlash:
					case ItemType1HBlunt:
					case ItemType1HPiercing:
					case ItemTypeMartial:
					case ItemType2HSlash:
					case ItemType2HBlunt:
					case ItemType2HPiercing:
					case ItemTypeBow:
						break;
					default:
						restrictfail = true;
						break;
					}
					break;
				case AugRestr1HWeapons:
					switch(it) {
					case ItemType1HSlash:
					case ItemType1HBlunt:
					case ItemType1HPiercing:
					case ItemTypeMartial:
						break;
					default:
						restrictfail = true;
						break;
					}
					break;
				case AugRestr2HWeapons:
					switch(it) {
					case ItemType2HSlash:
					case ItemType2HBlunt:
					case ItemType2HPiercing:
					case ItemTypeBow:
						break;
					default:
						restrictfail = true;
						break;
					}
					break;
				case AugRestr1HSlash:
					switch(it) {
					case ItemType1HSlash:
						break;
					default:
						restrictfail = true;
						break;
					}
					break;
				case AugRestr1HBlunt:
					switch(it) {
					case ItemType1HBlunt:
						break;
					default:
						restrictfail = true;
						break;
					}
					break;
				case AugRestrPiercing:
					switch(it) {
					case ItemType1HPiercing:
						break;
					default:
						restrictfail = true;
						break;
					}
					break;
				case AugRestrHandToHand:
					switch(it) {
					case ItemTypeMartial:
						break;
					default:
						restrictfail = true;
						break;
					}
					break;
				case AugRestr2HSlash:
					switch(it) {
					case ItemType2HSlash:
						break;
					default:
						restrictfail = true;
						break;
					}
					break;
				case AugRestr2HBlunt:
					switch(it) {
					case ItemType2HBlunt:
						break;
					default:
						restrictfail = true;
						break;
					}
					break;
				case AugRestr2HPierce:
					switch(it) {
					case ItemType2HPiercing:
						break;
					default:
						restrictfail = true;
						break;
					}
					break;
				case AugRestrBows:
					switch(it) {
					case ItemTypeBow:
						break;
					default:
						restrictfail = true;
						break;
					}
					break;
				case AugRestrShields:
					switch(it) {
					case ItemTypeShield:
						break;
					default:
						restrictfail = true;
						break;
					}
					break;
				case AugRestr1HSlash1HBluntOrHandToHand:
					switch(it) {
					case ItemType1HSlash:
					case ItemType1HBlunt:
					case ItemTypeMartial:
						break;
					default:
						restrictfail = true;
						break;
					}
					break;
				case AugRestr1HBluntOrHandToHand:
					switch(it) {
					case ItemType1HBlunt:
					case ItemTypeMartial:
						break;
					default:
						restrictfail = true;
						break;
					}
					break;
				// These 3 are in-work
				case AugRestrUnknown1:
				case AugRestrUnknown2:
				case AugRestrUnknown3:
				default:
					restrictfail = true;
					break;
				}

				if(restrictfail) {
					Message(13, "Augment %u (Aug%i) is restricted from wear on Item %u.", augments[iter], (iter + 1), item->ID);
					mlog(INVENTORY__ERROR, "Player %s on account %s attempted to augment an item with a restricted augment (Aug%i).\n(Item: %u, Aug1: %u, Aug2: %u, Aug3: %u, Aug4: %u, Aug5: %u, Aug6: %u)\n",
						GetName(), account_name, (iter + 1), item->ID, aug1, aug2, aug3, aug4, aug5, aug6);

					return false;
				}
			}

			if(enforceusable) {
				// check for class usability
				if(item->Classes && !(classes &= augtest->Classes)) {
					Message(13, "Augment %u (Aug%i) will result in an item not usable by any class.", augments[iter], (iter + 1));
					mlog(INVENTORY__ERROR, "Player %s on account %s attempted to create an item unusable by any class.\n(Item: %u, Aug1: %u, Aug2: %u, Aug3: %u, Aug4: %u, Aug5: %u, Aug6: %u)\n",
						GetName(), account_name, item->ID, aug1, aug2, aug3, aug4, aug5, aug6);

					return false;
				}

				// check for race usability
				if(item->Races && !(races &= augtest->Races)) {
					Message(13, "Augment %u (Aug%i) will result in an item not usable by any race.", augments[iter], (iter + 1));
					mlog(INVENTORY__ERROR, "Player %s on account %s attempted to create an item unusable by any race.\n(Item: %u, Aug1: %u, Aug2: %u, Aug3: %u, Aug4: %u, Aug5: %u, Aug6: %u)\n",
						GetName(), account_name, item->ID, aug1, aug2, aug3, aug4, aug5, aug6);

					return false;
				}

				// check for slot usability
				if(item->Slots && !(slots &= augtest->Slots)) {
					Message(13, "Augment %u (Aug%i) will result in an item not usable in any slot.", augments[iter], (iter + 1));
					mlog(INVENTORY__ERROR, "Player %s on account %s attempted to create an item unusable in any slot.\n(Item: %u, Aug1: %u, Aug2: %u, Aug3: %u, Aug4: %u, Aug5: %u, Aug6: %u)\n",
						GetName(), account_name, item->ID, aug1, aug2, aug3, aug4, aug5, aug6);

					return false;
				}
			}
		}
	}

	// validation passed..so, set the charges and create the actual item

	// if the item is stackable and the charge amount is -1 or 0 then set to 1 charge.
	// removed && item->MaxCharges == 0 if -1 or 0 was passed max charges is irrelevant 
	if(charges <= 0 && item->Stackable)
		charges = 1;

	// if the charges is -1, then no charge value was passed in set to max charges
	else if(charges == -1)
		charges = item->MaxCharges;

	// in any other situation just use charges as passed

	ItemInst* inst = database.CreateItem(item, charges);

	if(inst == nullptr) {
		Message(13, "An unknown server error has occurred and your item was not created.");
		// this goes to logfile since this is a major error
		LogFile->write(EQEMuLog::Error, "Player %s on account %s encountered an unknown item creation error.\n(Item: %u, Aug1: %u, Aug2: %u, Aug3: %u, Aug4: %u, Aug5: %u, Aug6: %u)\n",
			GetName(), account_name, item->ID, aug1, aug2, aug3, aug4, aug5, aug6);

		return false;
	}

	// add any validated augments
	for (int iter = AUG_BEGIN; iter < EmuConstants::ITEM_COMMON_SIZE; ++iter) {
		if(augments[iter])
			inst->PutAugment(&database, iter, augments[iter]);
	}

	// attune item
	if(attuned && inst->GetItem()->Attuneable)
		inst->SetAttuned(true);
		
	inst->SetOrnamentIcon(ornament_icon);
	inst->SetOrnamentationIDFile(ornament_idfile);
	inst->SetOrnamentHeroModel(ornament_hero_model);

	// check to see if item is usable in requested slot
	if(enforceusable && (((to_slot >= MainCharm) && (to_slot <= MainAmmo)) || (to_slot == MainPowerSource))) {
		uint32 slottest = (to_slot == MainPowerSource) ? 22 : to_slot; // can't change '22' just yet...

		if(!(slots & ((uint32)1 << slottest))) {
			Message(0, "This item is not equipable at slot %u - moving to cursor.", to_slot);
			mlog(INVENTORY__ERROR, "Player %s on account %s attempted to equip an item unusable in slot %u - moved to cursor.\n(Item: %u, Aug1: %u, Aug2: %u, Aug3: %u, Aug4: %u, Aug5: %u, Aug6: %u)\n",
				GetName(), account_name, to_slot, item->ID, aug1, aug2, aug3, aug4, aug5, aug6);

			to_slot = MainCursor;
		}
	}

	// put item into inventory
	if (to_slot == MainCursor) {
		PushItemOnCursor(*inst);
		SendItemPacket(MainCursor, inst, ItemPacketSummonItem);
	}
	else {
		PutItemInInventory(to_slot, *inst, true);
	}

	safe_delete(inst);

	// discover item and any augments
	if((RuleB(Character, EnableDiscoveredItems)) && !GetGM()) {
		if(!IsDiscovered(item_id))
			DiscoverItem(item_id);
		/*
		// Augments should have been discovered prior to being placed on an item.
		for (int iter = AUG_BEGIN; iter < EmuConstants::ITEM_COMMON_SIZE; ++iter) {
			if(augments[iter] && !IsDiscovered(augments[iter]))
				DiscoverItem(augments[iter]);
		}
		*/
	}

	return true;
}

// Drop item from inventory to ground (generally only dropped from SLOT_CURSOR)
void Client::DropItem(int16 slot_id)
{
	if(GetInv().CheckNoDrop(slot_id) && RuleI(World, FVNoDropFlag) == 0 ||
		RuleI(Character, MinStatusForNoDropExemptions) < Admin() && RuleI(World, FVNoDropFlag) == 2) {
		database.SetHackerFlag(this->AccountName(), this->GetCleanName(), "Tried to drop an item on the ground that was nodrop!");
		GetInv().DeleteItem(slot_id);
		return;
	}

	// Take control of item in client inventory
	ItemInst *inst = m_inv.PopItem(slot_id);
	if(inst) {
		int i = parse->EventItem(EVENT_DROP_ITEM, this, inst, nullptr, "", 0);
		if(i != 0) {
			safe_delete(inst);
		}
	} else {
		// Item doesn't exist in inventory!
		Message(13, "Error: Item not found in slot %i", slot_id);
		return;
	}

	// Save client inventory change to database
	if (slot_id == MainCursor) {
		SendCursorBuffer();
		std::list<ItemInst*>::const_iterator s=m_inv.cursor_begin(),e=m_inv.cursor_end();
		database.SaveCursor(CharacterID(), s, e);
	} else {
		database.SaveInventory(CharacterID(), nullptr, slot_id);
	}

	if(!inst)
		return;

	// Package as zone object
	Object* object = new Object(this, inst);
	entity_list.AddObject(object, true);
	object->StartDecay();

	safe_delete(inst);
}

// Drop inst
void Client::DropInst(const ItemInst* inst)
{
	if (!inst) {
		// Item doesn't exist in inventory!
		Message(13, "Error: Item not found");
		return;
	}


	if (inst->GetItem()->NoDrop == 0)
	{
		Message(13, "This item is NODROP. Deleting.");
		return;
	}

	// Package as zone object
	Object* object = new Object(this, inst);
	entity_list.AddObject(object, true);
	object->StartDecay();
}

// Returns a slot's item ID (returns INVALID_ID if not found)
int32 Client::GetItemIDAt(int16 slot_id) {
	const ItemInst* inst = m_inv[slot_id];
	if (inst)
		return inst->GetItem()->ID;

	// None found
	return INVALID_ID;
}

// Returns an augment's ID that's in an item (returns INVALID_ID if not found)
// Pass in the slot ID of the item and which augslot you want to check (0-4)
int32 Client::GetAugmentIDAt(int16 slot_id, uint8 augslot) {
	const ItemInst* inst = m_inv[slot_id];
	if (inst)
		if (inst->GetAugmentItemID(augslot))
			return inst->GetAugmentItemID(augslot);

	// None found
	return INVALID_ID;
}

void Client::SendCursorBuffer() {
	// Temporary work-around for the RoF+ Client Buffer
	// Instead of dealing with client moving items in cursor buffer,
	// we can just send the next item in the cursor buffer to the cursor.
	if (GetClientVersion() >= EQClientRoF)
	{
		if (!GetInv().CursorEmpty())
		{
			const ItemInst* inst = GetInv().GetCursorItem();
			if (inst)
			{
				SendItemPacket(MainCursor, inst, ItemPacketSummonItem);
			}
		}
	}
}

// Remove item from inventory
void Client::DeleteItemInInventory(int16 slot_id, int8 quantity, bool client_update, bool update_db) {
	#if (EQDEBUG >= 5)
		LogFile->write(EQEMuLog::Debug, "DeleteItemInInventory(%i, %i, %s)", slot_id, quantity, (client_update) ? "true":"false");
	#endif

	// Added 'IsSlotValid(slot_id)' check to both segments of client packet processing.
	// - cursor queue slots were slipping through and crashing client
	if(!m_inv[slot_id]) {
		// Make sure the client deletes anything in this slot to match the server.
		if(client_update && IsValidSlot(slot_id)) {
			EQApplicationPacket* outapp;
			outapp = new EQApplicationPacket(OP_DeleteItem, sizeof(DeleteItem_Struct));
			DeleteItem_Struct* delitem	= (DeleteItem_Struct*)outapp->pBuffer;
			delitem->from_slot			= slot_id;
			delitem->to_slot			= 0xFFFFFFFF;
			delitem->number_in_stack	= 0xFFFFFFFF;
			QueuePacket(outapp);
			safe_delete(outapp);
		}
		return;
	}

	// start QS code
	if(RuleB(QueryServ, PlayerLogDeletes)) {
		uint16 delete_count = 0;

		if(m_inv[slot_id]) { delete_count += m_inv.GetItem(slot_id)->GetTotalItemCount(); }

		ServerPacket* qspack = new ServerPacket(ServerOP_QSPlayerLogDeletes, sizeof(QSPlayerLogDelete_Struct) + (sizeof(QSDeleteItems_Struct) * delete_count));
		QSPlayerLogDelete_Struct* qsaudit = (QSPlayerLogDelete_Struct*)qspack->pBuffer;
		uint16 parent_offset = 0;

		qsaudit->char_id	= character_id;
		qsaudit->stack_size = quantity;
		qsaudit->char_count = delete_count;

		qsaudit->items[parent_offset].char_slot = slot_id;
		qsaudit->items[parent_offset].item_id	= m_inv[slot_id]->GetID();
		qsaudit->items[parent_offset].charges	= m_inv[slot_id]->GetCharges();
		qsaudit->items[parent_offset].aug_1		= m_inv[slot_id]->GetAugmentItemID(1);
		qsaudit->items[parent_offset].aug_2		= m_inv[slot_id]->GetAugmentItemID(2);
		qsaudit->items[parent_offset].aug_3		= m_inv[slot_id]->GetAugmentItemID(3);
		qsaudit->items[parent_offset].aug_4		= m_inv[slot_id]->GetAugmentItemID(4);
		qsaudit->items[parent_offset].aug_5		= m_inv[slot_id]->GetAugmentItemID(5);

		if(m_inv[slot_id]->IsType(ItemClassContainer)) {
			for(uint8 bag_idx = SUB_BEGIN; bag_idx < m_inv[slot_id]->GetItem()->BagSlots; bag_idx++) {
				ItemInst* bagitem = m_inv[slot_id]->GetItem(bag_idx);

				if(bagitem) {
					int16 bagslot_id = Inventory::CalcSlotId(slot_id, bag_idx);

					qsaudit->items[++parent_offset].char_slot	= bagslot_id;
					qsaudit->items[parent_offset].item_id		= bagitem->GetID();
					qsaudit->items[parent_offset].charges		= bagitem->GetCharges();
					qsaudit->items[parent_offset].aug_1			= bagitem->GetAugmentItemID(1);
					qsaudit->items[parent_offset].aug_2			= bagitem->GetAugmentItemID(2);
					qsaudit->items[parent_offset].aug_3			= bagitem->GetAugmentItemID(3);
					qsaudit->items[parent_offset].aug_4			= bagitem->GetAugmentItemID(4);
					qsaudit->items[parent_offset].aug_5			= bagitem->GetAugmentItemID(5);
				}
			}
		}

		qspack->Deflate();
		if(worldserver.Connected()) { worldserver.SendPacket(qspack); }
		safe_delete(qspack);
	}
	// end QS code

	bool isDeleted = m_inv.DeleteItem(slot_id, quantity);

	const ItemInst* inst=nullptr;
	if (slot_id == MainCursor) {
		std::list<ItemInst*>::const_iterator s=m_inv.cursor_begin(),e=m_inv.cursor_end();
		if(update_db)
			database.SaveCursor(character_id, s, e);
	}
	else {
		// Save change to database
		inst = m_inv[slot_id];
		if(update_db)
			database.SaveInventory(character_id, inst, slot_id);
	}

	if(client_update && IsValidSlot(slot_id)) {
		EQApplicationPacket* outapp;
		if(inst) {
			if(!inst->IsStackable() && !isDeleted)
				// Non stackable item with charges = Item with clicky spell effect ? Delete a charge.
				outapp = new EQApplicationPacket(OP_DeleteCharge, sizeof(MoveItem_Struct));
			else
				// Stackable, arrows, etc ? Delete one from the stack
				outapp = new EQApplicationPacket(OP_DeleteItem, sizeof(MoveItem_Struct));

				DeleteItem_Struct* delitem	= (DeleteItem_Struct*)outapp->pBuffer;
				delitem->from_slot			= slot_id;
				delitem->to_slot			= 0xFFFFFFFF;
				delitem->number_in_stack	= 0xFFFFFFFF;
				for(int loop=0;loop<quantity;loop++)
					QueuePacket(outapp);
				safe_delete(outapp);
		}
		else {
			outapp = new EQApplicationPacket(OP_MoveItem, sizeof(MoveItem_Struct));
			MoveItem_Struct* delitem	= (MoveItem_Struct*)outapp->pBuffer;
			delitem->from_slot			= slot_id;
			delitem->to_slot			= 0xFFFFFFFF;
			delitem->number_in_stack	= 0xFFFFFFFF;
			QueuePacket(outapp);
			safe_delete(outapp);
		}
	}
}

bool Client::PushItemOnCursor(const ItemInst& inst, bool client_update)
{
	mlog(INVENTORY__SLOTS, "Putting item %s (%d) on the cursor", inst.GetItem()->Name, inst.GetItem()->ID);
	m_inv.PushCursor(inst);

	if (client_update) {
		SendItemPacket(MainCursor, &inst, ItemPacketSummonItem);
	}

	std::list<ItemInst*>::const_iterator s=m_inv.cursor_begin(),e=m_inv.cursor_end();
	return database.SaveCursor(CharacterID(), s, e);
}

// Puts an item into the person's inventory
// Any items already there will be removed from user's inventory
// (Also saves changes back to the database: this may be optimized in the future)
// client_update: Sends packet to client
bool Client::PutItemInInventory(int16 slot_id, const ItemInst& inst, bool client_update) {
	mlog(INVENTORY__SLOTS, "Putting item %s (%d) into slot %d", inst.GetItem()->Name, inst.GetItem()->ID, slot_id);

	if (slot_id == MainCursor)
		return PushItemOnCursor(inst, client_update);
	else
		m_inv.PutItem(slot_id, inst);

	if (client_update)
	{
		SendItemPacket(slot_id, &inst, ((slot_id == MainCursor) ? ItemPacketSummonItem : ItemPacketTrade));
		//SendWearChange(Inventory::CalcMaterialFromSlot(slot_id));
	}
		

	if (slot_id == MainCursor) {
		std::list<ItemInst*>::const_iterator s = m_inv.cursor_begin(), e = m_inv.cursor_end();
		return database.SaveCursor(this->CharacterID(), s, e);
	}
	else {
		return database.SaveInventory(this->CharacterID(), &inst, slot_id);
	}

	CalcBonuses();
}

void Client::PutLootInInventory(int16 slot_id, const ItemInst &inst, ServerLootItem_Struct** bag_item_data)
{
	mlog(INVENTORY__SLOTS, "Putting loot item %s (%d) into slot %d", inst.GetItem()->Name, inst.GetItem()->ID, slot_id);
	m_inv.PutItem(slot_id, inst);

	SendLootItemInPacket(&inst, slot_id);

	if (slot_id == MainCursor) {
		std::list<ItemInst*>::const_iterator s=m_inv.cursor_begin(),e=m_inv.cursor_end();
		database.SaveCursor(this->CharacterID(), s, e);
	} else
		database.SaveInventory(this->CharacterID(), &inst, slot_id);

	if(bag_item_data)	// bag contents
	{
		int16 interior_slot;
		// solar: our bag went into slot_id, now let's pack the contents in
		for(int i = SUB_BEGIN; i < EmuConstants::ITEM_CONTAINER_SIZE; i++)
		{
			if(bag_item_data[i] == nullptr)
				continue;
			const ItemInst *bagitem = database.CreateItem(bag_item_data[i]->item_id, bag_item_data[i]->charges, bag_item_data[i]->aug_1, bag_item_data[i]->aug_2, bag_item_data[i]->aug_3, bag_item_data[i]->aug_4, bag_item_data[i]->aug_5, bag_item_data[i]->aug_6, bag_item_data[i]->attuned);
			interior_slot = Inventory::CalcSlotId(slot_id, i);
			mlog(INVENTORY__SLOTS, "Putting bag loot item %s (%d) into slot %d (bag slot %d)", inst.GetItem()->Name, inst.GetItem()->ID, interior_slot, i);
			PutLootInInventory(interior_slot, *bagitem);
			safe_delete(bagitem);
		}
	}

	CalcBonuses();
}
bool Client::TryStacking(ItemInst* item, uint8 type, bool try_worn, bool try_cursor){
	if(!item || !item->IsStackable() || item->GetCharges()>=item->GetItem()->StackSize)
		return false;
	int16 i;
	uint32 item_id = item->GetItem()->ID;
	for (i = EmuConstants::GENERAL_BEGIN; i <= EmuConstants::GENERAL_END; i++)
	{
		ItemInst* tmp_inst = m_inv.GetItem(i);
		if(tmp_inst && tmp_inst->GetItem()->ID == item_id && tmp_inst->GetCharges() < tmp_inst->GetItem()->StackSize){
			MoveItemCharges(*item, i, type);
			CalcBonuses();
			if(item->GetCharges())	// we didn't get them all
				return AutoPutLootInInventory(*item, try_worn, try_cursor, 0);
			return true;
		}
	}
	for (i = EmuConstants::GENERAL_BEGIN; i <= EmuConstants::GENERAL_END; i++)
	{
		for (uint8 j = SUB_BEGIN; j < EmuConstants::ITEM_CONTAINER_SIZE; j++)
		{
			uint16 slotid = Inventory::CalcSlotId(i, j);
			ItemInst* tmp_inst = m_inv.GetItem(slotid);

			if(tmp_inst && tmp_inst->GetItem()->ID == item_id && tmp_inst->GetCharges() < tmp_inst->GetItem()->StackSize){
				MoveItemCharges(*item, slotid, type);
				CalcBonuses();
				if(item->GetCharges())	// we didn't get them all
					return AutoPutLootInInventory(*item, try_worn, try_cursor, 0);
				return true;
			}
		}
	}
	return false;
}
// Locate an available space in inventory to place an item
// and then put the item there
// The change will be saved to the database
bool Client::AutoPutLootInInventory(ItemInst& inst, bool try_worn, bool try_cursor, ServerLootItem_Struct** bag_item_data)
{
	// #1: Try to auto equip
	if (try_worn && inst.IsEquipable(GetBaseRace(), GetClass()) && inst.GetItem()->ReqLevel<=level && !inst.GetItem()->Attuneable && inst.GetItem()->ItemType != ItemTypeAugmentation)
	{
		// too messy as-is... <watch>
		for (int16 i = EmuConstants::EQUIPMENT_BEGIN; i < MainPowerSource; i++) // originally (i < 22)
		{
			if (i == EmuConstants::GENERAL_BEGIN) {
				if(this->GetClientVersion() >= EQClientSoF) { i = MainPowerSource; } // added power source check for SoF+ clients
				else { break; }
			}

			if (!m_inv[i])
			{
				if( i == MainPrimary && inst.IsWeapon() ) // If item is primary slot weapon
				{
					if( (inst.GetItem()->ItemType == ItemType2HSlash) || (inst.GetItem()->ItemType == ItemType2HBlunt) || (inst.GetItem()->ItemType == ItemType2HPiercing) ) // and uses 2hs \ 2hb \ 2hp
					{
						if( m_inv[MainSecondary] ) // and if secondary slot is not empty
						{
							continue; // Can't auto-equip
						}
					}
				}
				if( i== MainSecondary && m_inv[MainPrimary]) // check to see if primary slot is a two hander
				{
					uint8 use = m_inv[MainPrimary]->GetItem()->ItemType;
					if(use == ItemType2HSlash || use == ItemType2HBlunt || use == ItemType2HPiercing)
						continue;
				}
				if
				(
					i == MainSecondary &&
					inst.IsWeapon() &&
					!CanThisClassDualWield()
				)
				{
					continue;
				}

				if (inst.IsEquipable(i))	// Equippable at this slot?
				{
					//send worn to everyone...
					PutLootInInventory(i, inst);
					uint8 worn_slot_material = Inventory::CalcMaterialFromSlot(i);
					if(worn_slot_material != _MaterialInvalid)
					{
						SendWearChange(worn_slot_material);
					}
					
					parse->EventItem(EVENT_EQUIP_ITEM, this, &inst, nullptr, "", i);
					return true;
				}
			}
		}
	}

	// #2: Stackable item?
	if (inst.IsStackable())
	{
		if(TryStacking(&inst, ItemPacketTrade, try_worn, try_cursor))
			return true;
	}

	// #3: put it in inventory
	bool is_arrow = (inst.GetItem()->ItemType == ItemTypeArrow) ? true : false;
	int16 slot_id = m_inv.FindFreeSlot(inst.IsType(ItemClassContainer), try_cursor, inst.GetItem()->Size, is_arrow);
	if (slot_id != INVALID_INDEX)
	{
		PutLootInInventory(slot_id, inst, bag_item_data);
		return true;
	}

	return false;
}

// solar: helper function for AutoPutLootInInventory
void Client::MoveItemCharges(ItemInst &from, int16 to_slot, uint8 type)
{
	ItemInst *tmp_inst = m_inv.GetItem(to_slot);

	if(tmp_inst && tmp_inst->GetCharges() < tmp_inst->GetItem()->StackSize)
	{
		// this is how much room is left on the item we're stacking onto
		int charge_slots_left = tmp_inst->GetItem()->StackSize - tmp_inst->GetCharges();
		// this is how many charges we can move from the looted item to
		// the item in the inventory
		int charges_to_move =
			from.GetCharges() < charge_slots_left ?
				from.GetCharges() :
				charge_slots_left;

		tmp_inst->SetCharges(tmp_inst->GetCharges() + charges_to_move);
		from.SetCharges(from.GetCharges() - charges_to_move);
		SendLootItemInPacket(tmp_inst, to_slot);
		if (to_slot == MainCursor){
			std::list<ItemInst*>::const_iterator s=m_inv.cursor_begin(),e=m_inv.cursor_end();
			database.SaveCursor(this->CharacterID(), s, e);
		} else
			database.SaveInventory(this->CharacterID(), tmp_inst, to_slot);
	}
}

// TODO: needs clean-up to save references
bool MakeItemLink(char* &ret_link, const Item_Struct *item, uint32 aug0, uint32 aug1, uint32 aug2, uint32 aug3, uint32 aug4, uint32 aug5, uint8 evolving, uint8 evolvedlevel) {
	//we're sending back the entire "link", minus the null characters & item name
	//that way, we can use it for regular links & Task links
	//note: initiator needs to pass us ret_link

	/*
	--- Usage ---
	Chat: "%c" "%s" "%s" "%c", 0x12, ret_link, inst->GetItem()->name, 0x12
	Task: "<a WndNotify=\"27," "%s" "\">" "%s" "</a>", ret_link, inst->GetItem()->name
	<a WndNotify="27,00960F000000000000000000000000000000000000000">Master's Book of Wood Elven Culture</a>
	http://eqitems.13th-floor.org/phpBB2/viewtopic.php?p=510#510
	*/

	if (!item) //have to have an item to make the link
		return false;

	//format:
	//0	itemid	aug1	aug2	aug3	aug4	aug5	evolving?	loregroup	evolved level	hash
	//0	00000	00000	00000	00000	00000	00000	0			0000		0				00000000
	//length:
	//1	5		5		5		5		5		5		1			4			1				8		= 45
	//evolving item info: http://eqitems.13th-floor.org/phpBB2/viewtopic.php?t=145#558

	//int hash = GetItemLinkHash(inst);	//eventually this will work (currently crashes zone), but for now we'll skip the extra overhead
	int hash = NOT_USED;
	
	// Tested with UF and RoF..there appears to be a problem with using non-augment arguments below...
	// Currently, enabling them causes misalignments in what the client expects. I haven't looked
	// into it further to determine the cause..but, the function is setup to accept the parameters.
	// Note: some links appear with '00000' in front of the name..so, it's likely we need to send
	// some additional information when certain parameters are true -U
	//switch (GetClientVersion()) {
	switch (0) {
	case EQClientRoF2:
		// This operator contains 14 parameter masks..but, only 13 parameter values.
		// Even so, the client link appears ok... Need to figure out the discrepancy -U
		MakeAnyLenString(&ret_link, "%1X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%1X" "%1X" "%04X" "%1X" "%05X" "%08X",
			0,
			item->ID,
			aug0,
			aug1,
			aug2,
			aug3,
			aug4,
			aug5,
			0,//evolving,
			0,//item->LoreGroup,
			0,//evolvedlevel,
			0,
			hash
			);
		return true;
	case EQClientRoF:
		MakeAnyLenString(&ret_link, "%1X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%1X" "%04X" "%1X" "%05X" "%08X",
			0,
			item->ID,
			aug0,
			aug1,
			aug2,
			aug3,
			aug4,
			aug5,
			0,//evolving,
			0,//item->LoreGroup,
			0,//evolvedlevel,
			0,
			hash
			);
		return true;
	case EQClientUnderfoot:
	case EQClientSoD:
	case EQClientSoF:
		MakeAnyLenString(&ret_link, "%1X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%1X" "%04X" "%1X" "%05X" "%08X",
			0,
			item->ID,
			aug0,
			aug1,
			aug2,
			aug3,
			aug4,
			0,//evolving,
			0,//item->LoreGroup,
			0,//evolvedlevel,
			0,
			hash
			);
		return true;
	case EQClientTitanium:
		MakeAnyLenString(&ret_link, "%1X" "%05X" "%05X" "%05X" "%05X" "%05X" "%05X" "%1X" "%04X" "%1X" "%08X",
			0,
			item->ID,
			aug0,
			aug1,
			aug2,
			aug3,
			aug4,
			0,//evolving,
			0,//item->LoreGroup,
			0,//evolvedlevel,
			hash
			);
		return true;
	case EQClient62:
	default:
		return false;
	}
}

int Client::GetItemLinkHash(const ItemInst* inst) {
	//pre-Titanium: http://eqitems.13th-floor.org/phpBB2/viewtopic.php?t=70&postdays=0&postorder=asc
	//Titanium: http://eqitems.13th-floor.org/phpBB2/viewtopic.php?t=145
	if (!inst)	//have to have an item to make the hash
		return 0;

	const Item_Struct* item = inst->GetItem();
	char* hash_str = 0;
	/*register */int hash = 0;

	//now the fun part, since different types of items use different hashes...
	if (item->ItemClass == 0 && item->CharmFileID) {	//charm
		MakeAnyLenString(&hash_str, "%d%s-1-1-1-1-1%d %d %d %d %d %d %d %d %d",
			item->ID,
			item->Name,
			item->Light,
			item->Icon,
			item->Price,
			item->Size,
			item->Weight,
			item->ItemClass,
			item->ItemType,
			item->Favor,
			item->GuildFavor);
	} else if (item->ItemClass == 2) {	//book
		MakeAnyLenString(&hash_str, "%d%s%d%d%09X",
			item->ID,
			item->Name,
			item->Weight,
			item->BookType,
			item->Price);
	} else if (item->ItemClass == 1) {	//bag
		MakeAnyLenString(&hash_str, "%d%s%x%d%09X%d",
			item->ID,
			item->Name,
			item->BagSlots,
			item->BagWR,
			item->Price,
			item->Weight);
	} else {	//everything else
		MakeAnyLenString(&hash_str, "%d%s-1-1-1-1-1%d %d %d %d %d %d %d %d %d %d %d %d %d",
			item->ID,
			item->Name,
			item->Mana,
			item->HP,
			item->Favor,
			item->Light,
			item->Icon,
			item->Price,
			item->Weight,
			item->ReqLevel,
			item->Size,
			item->ItemClass,
			item->ItemType,
			item->AC,
			item->GuildFavor);
	}

	//this currently crashes zone, so someone feel free to fix this so we can work with hashes:
	//*** glibc detected *** double free or corruption (out): 0xb2403470 ***

	/*
	while (*hash_str != '\0') {
		register int c = toupper(*hash_str);

		asm volatile("\
			imul $31, %1, %1;\
			movzx %%ax, %%edx;\
			addl %%edx, %1;\
			movl %1, %0;\
			"
			:"=r"(hash)
			:"D"(hash), "a"(c)
			:"%edx"
			);

		// This is what the inline asm is doing:
		// hash *= 0x1f;
		// hash += (int)c;

		hash_str++;
	}
	*/

	safe_delete_array(hash_str);
	return hash;
}

// This appears to still be in use... The core of this should be incorporated into class Client::TextLink
void Client::SendItemLink(const ItemInst* inst, bool send_to_all)
{
/*

this stuff is old, live dosent do this anymore. they send a much smaller
packet with the item number in it, but I cant seem to find it right now

*/
	if (!inst)
		return;

	const Item_Struct* item = inst->GetItem();
	const char* name2 = &item->Name[0];
	EQApplicationPacket* outapp = new EQApplicationPacket(OP_ItemLinkText,strlen(name2)+68);
	char buffer2[135] = {0};
	char itemlink[135] = {0};
	sprintf(itemlink,"%c0%06u0%05u-%05u-%05u-%05u-%05u00000000%c",
		0x12,
		item->ID,
		inst->GetAugmentItemID(0),
		inst->GetAugmentItemID(1),
		inst->GetAugmentItemID(2),
		inst->GetAugmentItemID(3),
		inst->GetAugmentItemID(4),
		0x12);
	sprintf(buffer2,"%c%c%c%c%c%c%c%c%c%c%c%c%s",0x00,0x00,0x00,0x00,0xD3,0x01,0x00,0x00,0x1E,0x01,0x00,0x00,itemlink);
	memcpy(outapp->pBuffer,buffer2,outapp->size);
	QueuePacket(outapp);
	safe_delete(outapp);
	if (send_to_all==false)
		return;
	const char* charname = this->GetName();
	outapp = new EQApplicationPacket(OP_ItemLinkText,strlen(itemlink)+14+strlen(charname));
	char buffer3[150] = {0};
	sprintf(buffer3,"%c%c%c%c%c%c%c%c%c%c%c%c%6s%c%s",0x00,0x00,0x00,0x00,0xD2,0x01,0x00,0x00,0x00,0x00,0x00,0x00,charname,0x00,itemlink);
	memcpy(outapp->pBuffer,buffer3,outapp->size);
	entity_list.QueueCloseClients(this->CastToMob(),outapp,true,200,0,false);
	safe_delete(outapp);
}

void Client::SendLootItemInPacket(const ItemInst* inst, int16 slot_id)
{
	SendItemPacket(slot_id,inst, ItemPacketTrade);
}

bool Client::IsValidSlot(uint32 slot) {
	if ((slot == (uint32)INVALID_INDEX) ||
		(slot >= MAIN_BEGIN && slot < EmuConstants::MAP_POSSESSIONS_SIZE) ||
		(slot >= EmuConstants::GENERAL_BAGS_BEGIN && slot <= EmuConstants::CURSOR_BAG_END) ||
		(slot >= EmuConstants::TRIBUTE_BEGIN && slot <= EmuConstants::TRIBUTE_END) ||
		(slot >= EmuConstants::BANK_BEGIN && slot <= EmuConstants::BANK_END) ||
		(slot >= EmuConstants::BANK_BAGS_BEGIN && slot <= EmuConstants::BANK_BAGS_END) ||
		(slot >= EmuConstants::SHARED_BANK_BEGIN && slot <= EmuConstants::SHARED_BANK_END) ||
		(slot >= EmuConstants::SHARED_BANK_BAGS_BEGIN && slot <= EmuConstants::SHARED_BANK_BAGS_END) ||
		(slot >= EmuConstants::TRADE_BEGIN && slot <= EmuConstants::TRADE_END) ||
		(slot >= EmuConstants::WORLD_BEGIN && slot <= EmuConstants::WORLD_END) ||
		(slot == MainPowerSource))
		return true;
	else
		return false;
}

bool Client::IsBankSlot(uint32 slot)
{
	if ((slot >= EmuConstants::BANK_BEGIN && slot <= EmuConstants::BANK_END) ||
		(slot >= EmuConstants::BANK_BAGS_BEGIN && slot <= EmuConstants::BANK_BAGS_END) ||
		(slot >= EmuConstants::SHARED_BANK_BEGIN && slot <= EmuConstants::SHARED_BANK_END) ||
		(slot >= EmuConstants::SHARED_BANK_BAGS_BEGIN && slot <= EmuConstants::SHARED_BANK_BAGS_END))
	{
		return true;
	}

	return false;
}

// Moves items around both internally and in the database
// In the future, this can be optimized by pushing all changes through one database REPLACE call
bool Client::SwapItem(MoveItem_Struct* move_in) {

	uint32 src_slot_check = move_in->from_slot;
	uint32 dst_slot_check = move_in->to_slot;
	uint32 stack_count_check = move_in->number_in_stack;

	if(!IsValidSlot(src_slot_check)){
		// SoF+ sends a Unix timestamp (should be int32) for src and dst slots every 10 minutes for some reason.
		if(src_slot_check < 2147483647)
			Message(13, "Warning: Invalid slot move from slot %u to slot %u with %u charges!", src_slot_check, dst_slot_check, stack_count_check);
		mlog(INVENTORY__SLOTS, "Invalid slot move from slot %u to slot %u with %u charges!", src_slot_check, dst_slot_check, stack_count_check);
		return false;
	}

	if(!IsValidSlot(dst_slot_check)) {
		// SoF+ sends a Unix timestamp (should be int32) for src and dst slots every 10 minutes for some reason.
		if(src_slot_check < 2147483647)
			Message(13, "Warning: Invalid slot move from slot %u to slot %u with %u charges!", src_slot_check, dst_slot_check, stack_count_check);
		mlog(INVENTORY__SLOTS, "Invalid slot move from slot %u to slot %u with %u charges!", src_slot_check, dst_slot_check, stack_count_check);
		return false;
	}

	// This could be expounded upon at some point to let the server know that
	// the client has moved a buffered cursor item onto the active cursor -U
	if (move_in->from_slot == move_in->to_slot) { // Item summon, no further processing needed
		if(RuleB(QueryServ, PlayerLogMoves)) { QSSwapItemAuditor(move_in); } // QS Audit
		return true;
	}

	if (move_in->to_slot == (uint32)INVALID_INDEX) {
		if (move_in->from_slot == (uint32)MainCursor) {
			mlog(INVENTORY__SLOTS, "Client destroyed item from cursor slot %d", move_in->from_slot);
			if(RuleB(QueryServ, PlayerLogMoves)) { QSSwapItemAuditor(move_in); } // QS Audit

			ItemInst *inst = m_inv.GetItem(MainCursor);
			if(inst) {
				parse->EventItem(EVENT_DESTROY_ITEM, this, inst, nullptr, "", 0);
			}

			DeleteItemInInventory(move_in->from_slot);
			SendCursorBuffer();

			return true; // Item destroyed by client
		}
		else {
			mlog(INVENTORY__SLOTS, "Deleted item from slot %d as a result of an inventory container tradeskill combine.", move_in->from_slot);
			if(RuleB(QueryServ, PlayerLogMoves)) { QSSwapItemAuditor(move_in); } // QS Audit
			DeleteItemInInventory(move_in->from_slot);
			return true; // Item deletion
		}
	}
	if(auto_attack && (move_in->from_slot == MainPrimary || move_in->from_slot == MainSecondary || move_in->from_slot == MainRange))
		SetAttackTimer();
	else if(auto_attack && (move_in->to_slot == MainPrimary || move_in->to_slot == MainSecondary || move_in->to_slot == MainRange))
		SetAttackTimer();
	// Step 1: Variables
	int16 src_slot_id = (int16)move_in->from_slot;
	int16 dst_slot_id = (int16)move_in->to_slot;

	if(IsBankSlot(src_slot_id) ||
		IsBankSlot(dst_slot_id) ||
		IsBankSlot(src_slot_check) ||
		IsBankSlot(dst_slot_check))
	{
		uint32 distance = 0;
		NPC *banker = entity_list.GetClosestBanker(this, distance);

		if(!banker || distance > USE_NPC_RANGE2)
		{
			char *hacked_string = nullptr;
			MakeAnyLenString(&hacked_string, "Player tried to make use of a banker(items) but %s is non-existant or too far away (%u units).",
				banker ? banker->GetName() : "UNKNOWN NPC", distance);
			database.SetMQDetectionFlag(AccountName(), GetName(), hacked_string, zone->GetShortName());
			safe_delete_array(hacked_string);
			Kick();	// Kicking player to avoid item loss do to client and server inventories not being sync'd
			return false;
		}
	}

	//Setup
	uint32 srcitemid = 0;
	uint32 dstitemid = 0;
	ItemInst* src_inst = m_inv.GetItem(src_slot_id);
	ItemInst* dst_inst = m_inv.GetItem(dst_slot_id);
	if (src_inst){
		mlog(INVENTORY__SLOTS, "Src slot %d has item %s (%d) with %d charges in it.", src_slot_id, src_inst->GetItem()->Name, src_inst->GetItem()->ID, src_inst->GetCharges());
		srcitemid = src_inst->GetItem()->ID;
		//SetTint(dst_slot_id,src_inst->GetColor());
		if (src_inst->GetCharges() > 0 && (src_inst->GetCharges() < (int16)move_in->number_in_stack || move_in->number_in_stack > src_inst->GetItem()->StackSize))
		{
			Message(13,"Error: Insufficient number in stack.");
			return false;
		}
	}
	if (dst_inst) {
		mlog(INVENTORY__SLOTS, "Dest slot %d has item %s (%d) with %d charges in it.", dst_slot_id, dst_inst->GetItem()->Name, dst_inst->GetItem()->ID, dst_inst->GetCharges());
		dstitemid = dst_inst->GetItem()->ID;
	}
	if (Trader && srcitemid>0){
		ItemInst* srcbag;
		ItemInst* dstbag;
		uint32 srcbagid =0;
		uint32 dstbagid = 0;

		//if (src_slot_id >= 250 && src_slot_id < 330) {
		if (src_slot_id >= EmuConstants::GENERAL_BAGS_BEGIN && src_slot_id <= EmuConstants::GENERAL_BAGS_END) {
			srcbag = m_inv.GetItem(((int)(src_slot_id / 10)) - 3);
			if (srcbag)
				srcbagid = srcbag->GetItem()->ID;
		}
		//if (dst_slot_id >= 250 && dst_slot_id < 330) {
		if (dst_slot_id >= EmuConstants::GENERAL_BAGS_BEGIN && dst_slot_id <= EmuConstants::GENERAL_BAGS_END) {
			dstbag = m_inv.GetItem(((int)(dst_slot_id / 10)) - 3);
			if (dstbag)
				dstbagid = dstbag->GetItem()->ID;
		}
		if (srcitemid==17899 || srcbagid==17899 || dstitemid==17899 || dstbagid==17899){
			this->Trader_EndTrader();
			this->Message(13,"You cannot move your Trader Satchels, or items inside them, while Trading.");
		}
	}

	// Step 2: Validate item in from_slot
	// After this, we can assume src_inst is a valid ptr
	if (!src_inst && (src_slot_id < EmuConstants::WORLD_BEGIN || src_slot_id > EmuConstants::WORLD_END)) {
		if (dst_inst) {
			// If there is no source item, but there is a destination item,
			// move the slots around before deleting the invalid source slot item,
			// which is now in the destination slot.
			move_in->from_slot = dst_slot_check;
			move_in->to_slot = src_slot_check;
			move_in->number_in_stack = dst_inst->GetCharges();
			if(!SwapItem(move_in)) { mlog(INVENTORY__ERROR, "Recursive SwapItem call failed due to non-existent destination item (charid: %i, fromslot: %i, toslot: %i)", CharacterID(), src_slot_id, dst_slot_id); }
		}

		return false;
	}
	//verify shared bank transactions in the database
	if(src_inst && src_slot_id >= EmuConstants::SHARED_BANK_BEGIN && src_slot_id <= EmuConstants::SHARED_BANK_BAGS_END) {
		if(!database.VerifyInventory(account_id, src_slot_id, src_inst)) {
			LogFile->write(EQEMuLog::Error, "Player %s on account %s was found exploiting the shared bank.\n", GetName(), account_name);
			DeleteItemInInventory(dst_slot_id,0,true);
			return(false);
		}
		if(src_slot_id >= EmuConstants::SHARED_BANK_BEGIN && src_slot_id <= EmuConstants::SHARED_BANK_END && src_inst->IsType(ItemClassContainer)){
			for (uint8 idx = SUB_BEGIN; idx < EmuConstants::ITEM_CONTAINER_SIZE; idx++) {
				const ItemInst* baginst = src_inst->GetItem(idx);
				if(baginst && !database.VerifyInventory(account_id, Inventory::CalcSlotId(src_slot_id, idx), baginst)){
					DeleteItemInInventory(Inventory::CalcSlotId(src_slot_id, idx),0,false);
				}
			}
		}
	}
	if(dst_inst && dst_slot_id >= EmuConstants::SHARED_BANK_BEGIN && dst_slot_id <= EmuConstants::SHARED_BANK_BAGS_END) {
		if(!database.VerifyInventory(account_id, dst_slot_id, dst_inst)) {
			LogFile->write(EQEMuLog::Error, "Player %s on account %s was found exploting the shared bank.\n", GetName(), account_name);
			DeleteItemInInventory(src_slot_id,0,true);
			return(false);
		}
		if(dst_slot_id >= EmuConstants::SHARED_BANK_BEGIN && dst_slot_id <= EmuConstants::SHARED_BANK_END && dst_inst->IsType(ItemClassContainer)){
			for (uint8 idx = SUB_BEGIN; idx < EmuConstants::ITEM_CONTAINER_SIZE; idx++) {
				const ItemInst* baginst = dst_inst->GetItem(idx);
				if(baginst && !database.VerifyInventory(account_id, Inventory::CalcSlotId(dst_slot_id, idx), baginst)){
					DeleteItemInInventory(Inventory::CalcSlotId(dst_slot_id, idx),0,false);
				}
			}
		}
	}


	// Check for No Drop Hacks
	Mob* with = trade->With();
	if (((with && with->IsClient() && dst_slot_id >= EmuConstants::TRADE_BEGIN && dst_slot_id <= EmuConstants::TRADE_END) ||
	(dst_slot_id >= EmuConstants::SHARED_BANK_BEGIN && dst_slot_id <= EmuConstants::SHARED_BANK_BAGS_END))
	&& GetInv().CheckNoDrop(src_slot_id)
	&& RuleI(World, FVNoDropFlag) == 0 || RuleI(Character, MinStatusForNoDropExemptions) < Admin() && RuleI(World, FVNoDropFlag) == 2) {
		DeleteItemInInventory(src_slot_id);
		WorldKick();
		return false;
	}

	// Step 3: Check for interaction with World Container (tradeskills)
	if(m_tradeskill_object != nullptr) {
		if (src_slot_id >= EmuConstants::WORLD_BEGIN && src_slot_id <= EmuConstants::WORLD_END) {
			// Picking up item from world container
			ItemInst* inst = m_tradeskill_object->PopItem(Inventory::CalcBagIdx(src_slot_id));
			if (inst) {
				PutItemInInventory(dst_slot_id, *inst, false);
				safe_delete(inst);
			}

			if(RuleB(QueryServ, PlayerLogMoves)) { QSSwapItemAuditor(move_in, true); } // QS Audit

			return true;
		}
		else if (dst_slot_id >= EmuConstants::WORLD_BEGIN && dst_slot_id <= EmuConstants::WORLD_END) {
			// Putting item into world container, which may swap (or pile onto) with existing item
			uint8 world_idx = Inventory::CalcBagIdx(dst_slot_id);
			ItemInst* world_inst = m_tradeskill_object->PopItem(world_idx);

			// Case 1: No item in container, unidirectional "Put"
			if (world_inst == nullptr) {
				m_tradeskill_object->PutItem(world_idx, src_inst);
				m_inv.DeleteItem(src_slot_id);
			}
			else {
				const Item_Struct* world_item = world_inst->GetItem();
				const Item_Struct* src_item = src_inst->GetItem();
				if (world_item && src_item) {
					// Case 2: Same item on cursor, stacks, transfer of charges needed
					if ((world_item->ID == src_item->ID) && src_inst->IsStackable()) {
						int16 world_charges = world_inst->GetCharges();
						int16 src_charges = src_inst->GetCharges();

						// Fill up destination stack as much as possible
						world_charges += src_charges;
						if (world_charges > world_inst->GetItem()->StackSize) {
							src_charges = world_charges - world_inst->GetItem()->StackSize;
							world_charges = world_inst->GetItem()->StackSize;
						}
						else {
							src_charges = 0;
						}

						world_inst->SetCharges(world_charges);
						m_tradeskill_object->PutItem(world_idx, world_inst);
						m_tradeskill_object->Save();

						if (src_charges == 0) {
							m_inv.DeleteItem(src_slot_id); // DB remove will occur below
						}
						else {
							src_inst->SetCharges(src_charges);
						}
					}
					else {
						// Case 3: Swap the item on user with item in world container
						// World containers don't follow normal rules for swapping
						ItemInst* inv_inst = m_inv.PopItem(src_slot_id);
						m_tradeskill_object->PutItem(world_idx, inv_inst);
						m_inv.PutItem(src_slot_id, *world_inst);
						safe_delete(inv_inst);
					}
				}
			}

			safe_delete(world_inst);
			if (src_slot_id == MainCursor)
			{
				if (dstitemid == 0)
				{
					SendCursorBuffer();
				}
				std::list<ItemInst*>::const_iterator s = m_inv.cursor_begin(), e = m_inv.cursor_end();
				database.SaveCursor(character_id, s, e);
			}
			else
			{
				database.SaveInventory(character_id, m_inv[src_slot_id], src_slot_id);
			}

			if(RuleB(QueryServ, PlayerLogMoves)) { QSSwapItemAuditor(move_in, true); } // QS Audit

			return true;
		}
	}

	// Step 4: Check for entity trade
	if (dst_slot_id >= EmuConstants::TRADE_BEGIN && dst_slot_id <= EmuConstants::TRADE_END) {
		if (src_slot_id != MainCursor) {
			Kick();
			return false;
		}
		if (with) {
			mlog(INVENTORY__SLOTS, "Trade item move from slot %d to slot %d (trade with %s)", src_slot_id, dst_slot_id, with->GetName());
			// Fill Trade list with items from cursor
			if (!m_inv[MainCursor]) {
				Message(13, "Error: Cursor item not located on server!");
				return false;
			}

			// Add cursor item to trade bucket
			// Also sends trade information to other client of trade session
			if(RuleB(QueryServ, PlayerLogMoves)) { QSSwapItemAuditor(move_in); } // QS Audit

			trade->AddEntity(dst_slot_id, move_in->number_in_stack);
			if (dstitemid == 0)
			{
				SendCursorBuffer();
			}

			return true;
		} else {
			if(RuleB(QueryServ, PlayerLogMoves)) { QSSwapItemAuditor(move_in); } // QS Audit

			SummonItem(src_inst->GetID(), src_inst->GetCharges());
			DeleteItemInInventory(MainCursor);

			return true;
		}
	}

	bool all_to_stack = false;
	// Step 5: Swap (or stack) items
	if (move_in->number_in_stack > 0) {
		// Determine if charged items can stack
		if(src_inst && !src_inst->IsStackable()) {
			mlog(INVENTORY__ERROR, "Move from %d to %d with stack size %d. %s is not a stackable item. (charname: %s)", src_slot_id, dst_slot_id, move_in->number_in_stack, src_inst->GetItem()->Name, GetName());
			return false;
		}

		if (dst_inst) {
			if(src_inst->GetID() != dst_inst->GetID()) {
				mlog(INVENTORY__ERROR, "Move from %d to %d with stack size %d. Incompatible item types: %d != %d", src_slot_id, dst_slot_id, move_in->number_in_stack, src_inst->GetID(), dst_inst->GetID());
				return(false);
			}
			if(dst_inst->GetCharges() < dst_inst->GetItem()->StackSize) {
				//we have a chance of stacking.
				mlog(INVENTORY__SLOTS, "Move from %d to %d with stack size %d. dest has %d/%d charges", src_slot_id, dst_slot_id, move_in->number_in_stack, dst_inst->GetCharges(), dst_inst->GetItem()->StackSize);
				// Charges can be emptied into dst
				uint16 usedcharges = dst_inst->GetItem()->StackSize - dst_inst->GetCharges();
				if (usedcharges > move_in->number_in_stack)
					usedcharges = move_in->number_in_stack;

				dst_inst->SetCharges(dst_inst->GetCharges() + usedcharges);
				src_inst->SetCharges(src_inst->GetCharges() - usedcharges);

				// Depleted all charges?
				if (src_inst->GetCharges() < 1)
				{
					mlog(INVENTORY__SLOTS, "Dest (%d) now has %d charges, source (%d) was entirely consumed. (%d moved)", dst_slot_id, dst_inst->GetCharges(), src_slot_id, usedcharges);
					database.SaveInventory(CharacterID(),nullptr,src_slot_id);
					m_inv.DeleteItem(src_slot_id);
					all_to_stack = true;
				} else {
					mlog(INVENTORY__SLOTS, "Dest (%d) now has %d charges, source (%d) has %d (%d moved)", dst_slot_id, dst_inst->GetCharges(), src_slot_id, src_inst->GetCharges(), usedcharges);
				}
			} else {
				mlog(INVENTORY__ERROR, "Move from %d to %d with stack size %d. Exceeds dest maximum stack size: %d/%d", src_slot_id, dst_slot_id, move_in->number_in_stack, (src_inst->GetCharges()+dst_inst->GetCharges()), dst_inst->GetItem()->StackSize);
				return false;
			}
		}
		else {
			// Nothing in destination slot: split stack into two
			if ((int16)move_in->number_in_stack >= src_inst->GetCharges()) {
				// Move entire stack
				if(!m_inv.SwapItem(src_slot_id, dst_slot_id)) { return false; }
				mlog(INVENTORY__SLOTS, "Move entire stack from %d to %d with stack size %d. Dest empty.", src_slot_id, dst_slot_id, move_in->number_in_stack);
			}
			else {
				// Split into two
				src_inst->SetCharges(src_inst->GetCharges() - move_in->number_in_stack);
				mlog(INVENTORY__SLOTS, "Split stack of %s (%d) from slot %d to %d with stack size %d. Src keeps %d.", src_inst->GetItem()->Name, src_inst->GetItem()->ID, src_slot_id, dst_slot_id, move_in->number_in_stack, src_inst->GetCharges());
				ItemInst* inst = database.CreateItem(src_inst->GetItem(), move_in->number_in_stack);
				m_inv.PutItem(dst_slot_id, *inst);
				safe_delete(inst);
			}
		}
	}
	else {
		// Not dealing with charges - just do direct swap
		if(src_inst && (dst_slot_id <= EmuConstants::EQUIPMENT_END || dst_slot_id == MainPowerSource) && dst_slot_id >= EmuConstants::EQUIPMENT_BEGIN) {
			if (src_inst->GetItem()->Attuneable) {
				src_inst->SetAttuned(true);
			}
			if (src_inst->IsAugmented()) {
				for (int i = AUG_BEGIN; i < EmuConstants::ITEM_COMMON_SIZE; i++) {
					if (src_inst->GetAugment(i)) {
						if (src_inst->GetAugment(i)->GetItem()->Attuneable) {
							src_inst->GetAugment(i)->SetAttuned(true);
						}
					}
				}
			}
			SetMaterial(dst_slot_id,src_inst->GetItem()->ID);
		}
		if(!m_inv.SwapItem(src_slot_id, dst_slot_id)) { return false; }
		mlog(INVENTORY__SLOTS, "Moving entire item from slot %d to slot %d", src_slot_id, dst_slot_id);

		if(src_slot_id <= EmuConstants::EQUIPMENT_END || src_slot_id == MainPowerSource) {
			if(src_inst) {
				parse->EventItem(EVENT_UNEQUIP_ITEM, this, src_inst, nullptr, "", src_slot_id);
			}

			if(dst_inst) {
				parse->EventItem(EVENT_EQUIP_ITEM, this, dst_inst, nullptr, "", src_slot_id);
			}
		}

		if(dst_slot_id <= EmuConstants::EQUIPMENT_END || dst_slot_id == MainPowerSource) {
			if(dst_inst) {
				parse->EventItem(EVENT_UNEQUIP_ITEM, this, dst_inst, nullptr, "", dst_slot_id);
			}

			if(src_inst) {
				parse->EventItem(EVENT_EQUIP_ITEM, this, src_inst, nullptr, "", dst_slot_id);
			}
		}
	}

	int matslot = SlotConvert2(dst_slot_id);
	if (dst_slot_id <= EmuConstants::EQUIPMENT_END && matslot != MaterialHead) { // think this is to allow the client to update with /showhelm
		SendWearChange(matslot);
	}

	// Step 7: Save change to the database
	if (src_slot_id == MainCursor){
		// If not swapping another item to cursor and stacking items were depleted
		if (dstitemid == 0 || all_to_stack == true)
		{
			SendCursorBuffer();
		}
		std::list<ItemInst*>::const_iterator s=m_inv.cursor_begin(),e=m_inv.cursor_end();
		database.SaveCursor(character_id, s, e);
	} else
		database.SaveInventory(character_id, m_inv.GetItem(src_slot_id), src_slot_id);

	if (dst_slot_id == MainCursor) {
		std::list<ItemInst*>::const_iterator s=m_inv.cursor_begin(),e=m_inv.cursor_end();
		database.SaveCursor(character_id, s, e);
	} else
		database.SaveInventory(character_id, m_inv.GetItem(dst_slot_id), dst_slot_id);

	if(RuleB(QueryServ, PlayerLogMoves)) { QSSwapItemAuditor(move_in, true); } // QS Audit

	// Step 8: Re-calc stats
	CalcBonuses();
	return true;
}

void Client::SwapItemResync(MoveItem_Struct* move_slots) {
	// wow..this thing created a helluva memory leak...
	// with any luck..this won't be needed in the future

	// resync the 'from' and 'to' slots on an as-needed basis
	// Not as effective as the full process, but less intrusive to gameplay -U
	mlog(INVENTORY__ERROR, "Inventory desyncronization. (charname: %s, source: %i, destination: %i)", GetName(), move_slots->from_slot, move_slots->to_slot);
	Message(15, "Inventory Desyncronization detected: Resending slot data...");

	if((move_slots->from_slot >= EmuConstants::EQUIPMENT_BEGIN && move_slots->from_slot <= EmuConstants::CURSOR_BAG_END) || move_slots->from_slot == MainPowerSource) {
		int16 resync_slot = (Inventory::CalcSlotId(move_slots->from_slot) == INVALID_INDEX) ? move_slots->from_slot : Inventory::CalcSlotId(move_slots->from_slot);
		if (IsValidSlot(resync_slot) && resync_slot != INVALID_INDEX) {
			// This prevents the client from crashing when closing any 'phantom' bags -U
			const Item_Struct* token_struct = database.GetItem(22292); // 'Copper Coin'
			ItemInst* token_inst = database.CreateItem(token_struct, 1);

			SendItemPacket(resync_slot, token_inst, ItemPacketTrade);

			if(m_inv[resync_slot]) { SendItemPacket(resync_slot, m_inv[resync_slot], ItemPacketTrade); }
			else {
				EQApplicationPacket* outapp		= new EQApplicationPacket(OP_DeleteItem, sizeof(DeleteItem_Struct));
				DeleteItem_Struct* delete_slot	= (DeleteItem_Struct*)outapp->pBuffer;
				delete_slot->from_slot			= resync_slot;
				delete_slot->to_slot			= 0xFFFFFFFF;
				delete_slot->number_in_stack	= 0xFFFFFFFF;

				QueuePacket(outapp);
				safe_delete(outapp);
			}
			safe_delete(token_inst);
			Message(14, "Source slot %i resyncronized.", move_slots->from_slot);
		}
		else { Message(13, "Could not resyncronize source slot %i.", move_slots->from_slot); }
	}
	else {
		int16 resync_slot = (Inventory::CalcSlotId(move_slots->from_slot) == INVALID_INDEX) ? move_slots->from_slot : Inventory::CalcSlotId(move_slots->from_slot);
		if (IsValidSlot(resync_slot) && resync_slot != INVALID_INDEX) {
			if(m_inv[resync_slot]) {
				const Item_Struct* token_struct = database.GetItem(22292); // 'Copper Coin'
				ItemInst* token_inst = database.CreateItem(token_struct, 1);

				SendItemPacket(resync_slot, token_inst, ItemPacketTrade);
				SendItemPacket(resync_slot, m_inv[resync_slot], ItemPacketTrade);

				safe_delete(token_inst);
				Message(14, "Source slot %i resyncronized.", move_slots->from_slot);
			}
			else { Message(13, "Could not resyncronize source slot %i.", move_slots->from_slot); }
		}
		else { Message(13, "Could not resyncronize source slot %i.", move_slots->from_slot); }
	}

	if((move_slots->to_slot >= EmuConstants::EQUIPMENT_BEGIN && move_slots->to_slot <= EmuConstants::CURSOR_BAG_END) || move_slots->to_slot == MainPowerSource) {
		int16 resync_slot = (Inventory::CalcSlotId(move_slots->to_slot) == INVALID_INDEX) ? move_slots->to_slot : Inventory::CalcSlotId(move_slots->to_slot);
		if (IsValidSlot(resync_slot) && resync_slot != INVALID_INDEX) {
			const Item_Struct* token_struct = database.GetItem(22292); // 'Copper Coin'
			ItemInst* token_inst = database.CreateItem(token_struct, 1);

			SendItemPacket(resync_slot, token_inst, ItemPacketTrade);

			if(m_inv[resync_slot]) { SendItemPacket(resync_slot, m_inv[resync_slot], ItemPacketTrade); }
			else {
				EQApplicationPacket* outapp		= new EQApplicationPacket(OP_DeleteItem, sizeof(DeleteItem_Struct));
				DeleteItem_Struct* delete_slot	= (DeleteItem_Struct*)outapp->pBuffer;
				delete_slot->from_slot			= resync_slot;
				delete_slot->to_slot			= 0xFFFFFFFF;
				delete_slot->number_in_stack	= 0xFFFFFFFF;

				QueuePacket(outapp);
				safe_delete(outapp);
			}
			safe_delete(token_inst);
			Message(14, "Destination slot %i resyncronized.", move_slots->to_slot);
		}
		else { Message(13, "Could not resyncronize destination slot %i.", move_slots->to_slot); }
	}
	else {
		int16 resync_slot = (Inventory::CalcSlotId(move_slots->to_slot) == INVALID_INDEX) ? move_slots->to_slot : Inventory::CalcSlotId(move_slots->to_slot);
		if (IsValidSlot(resync_slot) && resync_slot != INVALID_INDEX) {
			if(m_inv[resync_slot]) {
				const Item_Struct* token_struct = database.GetItem(22292); // 'Copper Coin'
				ItemInst* token_inst = database.CreateItem(token_struct, 1);

				SendItemPacket(resync_slot, token_inst, ItemPacketTrade);
				SendItemPacket(resync_slot, m_inv[resync_slot], ItemPacketTrade);

				safe_delete(token_inst);
				Message(14, "Destination slot %i resyncronized.", move_slots->to_slot);
			}
			else { Message(13, "Could not resyncronize destination slot %i.", move_slots->to_slot); }
		}
		else { Message(13, "Could not resyncronize destination slot %i.", move_slots->to_slot); }
	}
}

void Client::QSSwapItemAuditor(MoveItem_Struct* move_in, bool postaction_call) {
	int16 from_slot_id = static_cast<int16>(move_in->from_slot);
	int16 to_slot_id	= static_cast<int16>(move_in->to_slot);
	int16 move_amount	= static_cast<int16>(move_in->number_in_stack);

	if(!m_inv[from_slot_id] && !m_inv[to_slot_id]) { return; }

	uint16 move_count = 0;

	if(m_inv[from_slot_id]) { move_count += m_inv[from_slot_id]->GetTotalItemCount(); }
	if(to_slot_id != from_slot_id) { if(m_inv[to_slot_id]) { move_count += m_inv[to_slot_id]->GetTotalItemCount(); } }

	ServerPacket* qspack = new ServerPacket(ServerOP_QSPlayerLogMoves, sizeof(QSPlayerLogMove_Struct) + (sizeof(QSMoveItems_Struct) * move_count));
	QSPlayerLogMove_Struct* qsaudit = (QSPlayerLogMove_Struct*)qspack->pBuffer;

	qsaudit->char_id	= character_id;
	qsaudit->stack_size = move_amount;
	qsaudit->char_count = move_count;
	qsaudit->postaction = postaction_call;
	qsaudit->from_slot	= from_slot_id;
	qsaudit->to_slot	= to_slot_id;

	move_count = 0;

	const ItemInst* from_inst = m_inv[postaction_call?to_slot_id:from_slot_id];

	if(from_inst) {
		qsaudit->items[move_count].from_slot	= from_slot_id;
		qsaudit->items[move_count].to_slot		= to_slot_id;
		qsaudit->items[move_count].item_id		= from_inst->GetID();
		qsaudit->items[move_count].charges		= from_inst->GetCharges();
		qsaudit->items[move_count].aug_1		= from_inst->GetAugmentItemID(1);
		qsaudit->items[move_count].aug_2		= from_inst->GetAugmentItemID(2);
		qsaudit->items[move_count].aug_3		= from_inst->GetAugmentItemID(3);
		qsaudit->items[move_count].aug_4		= from_inst->GetAugmentItemID(4);
		qsaudit->items[move_count++].aug_5		= from_inst->GetAugmentItemID(5);

		if(from_inst->IsType(ItemClassContainer)) {
			for(uint8 bag_idx = SUB_BEGIN; bag_idx < from_inst->GetItem()->BagSlots; bag_idx++) {
				const ItemInst* from_baginst = from_inst->GetItem(bag_idx);

				if(from_baginst) {
					qsaudit->items[move_count].from_slot	= Inventory::CalcSlotId(from_slot_id, bag_idx);
					qsaudit->items[move_count].to_slot		= Inventory::CalcSlotId(to_slot_id, bag_idx);
					qsaudit->items[move_count].item_id		= from_baginst->GetID();
					qsaudit->items[move_count].charges		= from_baginst->GetCharges();
					qsaudit->items[move_count].aug_1		= from_baginst->GetAugmentItemID(1);
					qsaudit->items[move_count].aug_2		= from_baginst->GetAugmentItemID(2);
					qsaudit->items[move_count].aug_3		= from_baginst->GetAugmentItemID(3);
					qsaudit->items[move_count].aug_4		= from_baginst->GetAugmentItemID(4);
					qsaudit->items[move_count++].aug_5		= from_baginst->GetAugmentItemID(5);
				}
			}
		}
	}

	if(to_slot_id != from_slot_id) {
		const ItemInst* to_inst = m_inv[postaction_call?from_slot_id:to_slot_id];

		if(to_inst) {
			qsaudit->items[move_count].from_slot	= to_slot_id;
			qsaudit->items[move_count].to_slot		= from_slot_id;
			qsaudit->items[move_count].item_id		= to_inst->GetID();
			qsaudit->items[move_count].charges		= to_inst->GetCharges();
			qsaudit->items[move_count].aug_1		= to_inst->GetAugmentItemID(1);
			qsaudit->items[move_count].aug_2		= to_inst->GetAugmentItemID(2);
			qsaudit->items[move_count].aug_3		= to_inst->GetAugmentItemID(3);
			qsaudit->items[move_count].aug_4		= to_inst->GetAugmentItemID(4);
			qsaudit->items[move_count++].aug_5		= to_inst->GetAugmentItemID(5);

			if(to_inst->IsType(ItemClassContainer)) {
				for(uint8 bag_idx = SUB_BEGIN; bag_idx < to_inst->GetItem()->BagSlots; bag_idx++) {
					const ItemInst* to_baginst = to_inst->GetItem(bag_idx);

					if(to_baginst) {
						qsaudit->items[move_count].from_slot	= Inventory::CalcSlotId(to_slot_id, bag_idx);
						qsaudit->items[move_count].to_slot		= Inventory::CalcSlotId(from_slot_id, bag_idx);
						qsaudit->items[move_count].item_id		= to_baginst->GetID();
						qsaudit->items[move_count].charges		= to_baginst->GetCharges();
						qsaudit->items[move_count].aug_1		= to_baginst->GetAugmentItemID(1);
						qsaudit->items[move_count].aug_2		= to_baginst->GetAugmentItemID(2);
						qsaudit->items[move_count].aug_3		= to_baginst->GetAugmentItemID(3);
						qsaudit->items[move_count].aug_4		= to_baginst->GetAugmentItemID(4);
						qsaudit->items[move_count++].aug_5		= to_baginst->GetAugmentItemID(5);
					}
				}
			}
		}
	}

	if(move_count && worldserver.Connected()) {
		qspack->Deflate();
		worldserver.SendPacket(qspack);
	}

	safe_delete(qspack);
}

void Client::DyeArmor(DyeStruct* dye){
	int16 slot=0;
	for (int i = EmuConstants::MATERIAL_BEGIN; i <= EmuConstants::MATERIAL_TINT_END; i++) {
		if(m_pp.item_tint[i].rgb.blue!=dye->dye[i].rgb.blue ||
			m_pp.item_tint[i].rgb.red!=dye->dye[i].rgb.red ||
			m_pp.item_tint[i].rgb.green != dye->dye[i].rgb.green){
			slot = m_inv.HasItem(32557, 1, invWherePersonal);
			if (slot != INVALID_INDEX){
				DeleteItemInInventory(slot,1,true);
				uint8 slot2=SlotConvert(i);
				ItemInst* inst = this->m_inv.GetItem(slot2);
				if(inst){
					uint32 armor_color = (dye->dye[i].rgb.red * 65536) + (dye->dye[i].rgb.green * 256) + (dye->dye[i].rgb.blue);
					inst->SetColor(armor_color); 
					database.SaveCharacterMaterialColor(this->CharacterID(), i, armor_color);
					database.SaveInventory(CharacterID(),inst,slot2);
					if(dye->dye[i].rgb.use_tint)
						m_pp.item_tint[i].rgb.use_tint = 0xFF;
					else
						m_pp.item_tint[i].rgb.use_tint=0x00;
				}
				m_pp.item_tint[i].rgb.blue=dye->dye[i].rgb.blue;
				m_pp.item_tint[i].rgb.red=dye->dye[i].rgb.red;
				m_pp.item_tint[i].rgb.green=dye->dye[i].rgb.green;
				SendWearChange(i);
			}
			else{
				Message(13,"Could not locate A Vial of Prismatic Dye.");
				return;
			}
		}
	}
	EQApplicationPacket* outapp=new EQApplicationPacket(OP_Dye,0);
	QueuePacket(outapp);
	safe_delete(outapp);
	
}

/*bool Client::DecreaseByItemType(uint32 type, uint8 amt) {
	const Item_Struct* TempItem = 0;
	ItemInst* ins;
	int x;
	for(x=EmuConstants::POSSESSIONS_BEGIN; x <= EmuConstants::POSSESSIONS_END; x++)
	{
		TempItem = 0;
		ins = GetInv().GetItem(x);
		if (ins)
			TempItem = ins->GetItem();
		if (TempItem && TempItem->ItemType == type)
		{
			if (ins->GetCharges() < amt)
			{
				amt -= ins->GetCharges();
				DeleteItemInInventory(x,amt,true);
			}
			else
			{
				DeleteItemInInventory(x,amt,true);
				amt = 0;
			}
			if (amt < 1)
				return true;
		}
	}
	for(x=EmuConstants::GENERAL_BAGS_BEGIN; x <= EmuConstants::GENERAL_BAGS_END; x++)
	{
		TempItem = 0;
		ins = GetInv().GetItem(x);
		if (ins)
			TempItem = ins->GetItem();
		if (TempItem && TempItem->ItemType == type)
		{
			if (ins->GetCharges() < amt)
			{
				amt -= ins->GetCharges();
				DeleteItemInInventory(x,amt,true);
			}
			else
			{
				DeleteItemInInventory(x,amt,true);
				amt = 0;
			}
			if (amt < 1)
				return true;
		}
	}
	return false;
}*/

bool Client::DecreaseByID(uint32 type, uint8 amt) {
	const Item_Struct* TempItem = 0;
	ItemInst* ins = nullptr;
	int x;
	int num = 0;
	for(x = EmuConstants::EQUIPMENT_BEGIN; x <= EmuConstants::GENERAL_BAGS_END; x++)
	{
		if (x == MainCursor + 1)
			x = EmuConstants::GENERAL_BAGS_BEGIN;
		TempItem = 0;
		ins = GetInv().GetItem(x);
		if (ins)
			TempItem = ins->GetItem();
		if (TempItem && TempItem->ID == type)
		{
			num += ins->GetCharges();
			if (num >= amt)
				break;
		}
	}
	if (num < amt)
		return false;
	for(x = EmuConstants::EQUIPMENT_BEGIN; x <= EmuConstants::GENERAL_BAGS_END; x++) // should this be CURSOR_BAG_END?
	{
		if (x == MainCursor + 1)
			x = EmuConstants::GENERAL_BAGS_BEGIN;
		TempItem = 0;
		ins = GetInv().GetItem(x);
		if (ins)
			TempItem = ins->GetItem();
		if (TempItem && TempItem->ID == type)
		{
			if (ins->GetCharges() < amt)
			{
				amt -= ins->GetCharges();
				DeleteItemInInventory(x,amt,true);
			}
			else
			{
				DeleteItemInInventory(x,amt,true);
				amt = 0;
			}
			if (amt < 1)
				break;
		}
	}
	return true;
}

void Client::RemoveNoRent(bool client_update) {
	int16 slot_id = 0;

	// equipment
	for(slot_id = EmuConstants::EQUIPMENT_BEGIN; slot_id <= EmuConstants::EQUIPMENT_END; slot_id++) {
		const ItemInst* inst = m_inv[slot_id];
		if(inst && !inst->GetItem()->NoRent) {
			mlog(INVENTORY__SLOTS, "NoRent Timer Lapse: Deleting %s from slot %i", inst->GetItem()->Name, slot_id);
			DeleteItemInInventory(slot_id, 0, client_update);
		}
	}

	// general
	for (slot_id = EmuConstants::GENERAL_BEGIN; slot_id <= EmuConstants::GENERAL_END; slot_id++) {
		const ItemInst* inst = m_inv[slot_id];
		if (inst && !inst->GetItem()->NoRent) {
			mlog(INVENTORY__SLOTS, "NoRent Timer Lapse: Deleting %s from slot %i", inst->GetItem()->Name, slot_id);
			DeleteItemInInventory(slot_id, 0, client_update);
		}
	}

	// power source
	if (m_inv[MainPowerSource]) {
		const ItemInst* inst = m_inv[MainPowerSource];
		if (inst && !inst->GetItem()->NoRent) {
			mlog(INVENTORY__SLOTS, "NoRent Timer Lapse: Deleting %s from slot %i", inst->GetItem()->Name, MainPowerSource);
			DeleteItemInInventory(MainPowerSource, 0, (GetClientVersion() >= EQClientSoF) ? client_update : false); // Ti slot non-existent
		}
	}

	// containers
	for(slot_id = EmuConstants::GENERAL_BAGS_BEGIN; slot_id <= EmuConstants::CURSOR_BAG_END; slot_id++) {
		const ItemInst* inst = m_inv[slot_id];
		if(inst && !inst->GetItem()->NoRent) {
			mlog(INVENTORY__SLOTS, "NoRent Timer Lapse: Deleting %s from slot %i", inst->GetItem()->Name, slot_id);
			DeleteItemInInventory(slot_id, 0, client_update);
		}
	}

	// bank
	for(slot_id = EmuConstants::BANK_BEGIN; slot_id <= EmuConstants::BANK_END; slot_id++) {
		const ItemInst* inst = m_inv[slot_id];
		if(inst && !inst->GetItem()->NoRent) {
			mlog(INVENTORY__SLOTS, "NoRent Timer Lapse: Deleting %s from slot %i", inst->GetItem()->Name, slot_id);
			DeleteItemInInventory(slot_id, 0, false); // Can't delete from client Bank slots
		}
	}

	// bank containers
	for(slot_id = EmuConstants::BANK_BAGS_BEGIN; slot_id <= EmuConstants::BANK_BAGS_END; slot_id++) {
		const ItemInst* inst = m_inv[slot_id];
		if(inst && !inst->GetItem()->NoRent) {
			mlog(INVENTORY__SLOTS, "NoRent Timer Lapse: Deleting %s from slot %i", inst->GetItem()->Name, slot_id);
			DeleteItemInInventory(slot_id, 0, false); // Can't delete from client Bank Container slots
		}
	}

	// shared bank
	for(slot_id = EmuConstants::SHARED_BANK_BEGIN; slot_id <= EmuConstants::SHARED_BANK_END; slot_id++) {
		const ItemInst* inst = m_inv[slot_id];
		if(inst && !inst->GetItem()->NoRent) {
			mlog(INVENTORY__SLOTS, "NoRent Timer Lapse: Deleting %s from slot %i", inst->GetItem()->Name, slot_id);
			DeleteItemInInventory(slot_id, 0, false); // Can't delete from client Shared Bank slots
		}
	}

	// shared bank containers
	for(slot_id = EmuConstants::SHARED_BANK_BAGS_BEGIN; slot_id <= EmuConstants::SHARED_BANK_BAGS_END; slot_id++) {
		const ItemInst* inst = m_inv[slot_id];
		if(inst && !inst->GetItem()->NoRent) {
			mlog(INVENTORY__SLOTS, "NoRent Timer Lapse: Deleting %s from slot %i", inst->GetItem()->Name, slot_id);
			DeleteItemInInventory(slot_id, 0, false); // Can't delete from client Shared Bank Container slots
		}
	}

	// cursor & limbo
	if (!m_inv.CursorEmpty()) {
		std::list<ItemInst*> local;
		ItemInst* inst = nullptr;

		while (!m_inv.CursorEmpty()) {
			inst = m_inv.PopItem(MainCursor);
			if (inst)
				local.push_back(inst);
		}

		std::list<ItemInst*>::iterator iter = local.begin();
		while (iter != local.end()) {
			inst = *iter;
			// should probably put a check here for valid pointer..but, that was checked when the item was put into inventory -U
			if (!inst->GetItem()->NoRent)
				mlog(INVENTORY__SLOTS, "NoRent Timer Lapse: Deleting %s from `Limbo`", inst->GetItem()->Name);
			else
				m_inv.PushCursor(**iter);

			safe_delete(*iter);
			iter = local.erase(iter);
		}

		std::list<ItemInst*>::const_iterator s = m_inv.cursor_begin(), e = m_inv.cursor_end();
		database.SaveCursor(this->CharacterID(), s, e);
		local.clear();
	}
}

// Two new methods to alleviate perpetual login desyncs
void Client::RemoveDuplicateLore(bool client_update) {
	int16 slot_id = 0;

	// equipment
	for(slot_id = EmuConstants::EQUIPMENT_BEGIN; slot_id <= EmuConstants::EQUIPMENT_END; slot_id++) {
		ItemInst* inst = m_inv.PopItem(slot_id);
		if(inst) {
			if(CheckLoreConflict(inst->GetItem())) {
				mlog(INVENTORY__ERROR, "Lore Duplication Error: Deleting %s from slot %i", inst->GetItem()->Name, slot_id);
				database.SaveInventory(character_id, nullptr, slot_id);
			}
			else {
				m_inv.PutItem(slot_id, *inst);
			}
			safe_delete(inst);
		}
	}

	// general
	for (slot_id = EmuConstants::GENERAL_BEGIN; slot_id <= EmuConstants::GENERAL_END; slot_id++) {
		ItemInst* inst = m_inv.PopItem(slot_id);
		if (inst) {
			if (CheckLoreConflict(inst->GetItem())) {
				mlog(INVENTORY__ERROR, "Lore Duplication Error: Deleting %s from slot %i", inst->GetItem()->Name, slot_id);
				database.SaveInventory(character_id, nullptr, slot_id);
			}
			else {
				m_inv.PutItem(slot_id, *inst);
			}
			safe_delete(inst);
		}
	}

	// power source
	if (m_inv[MainPowerSource]) {
		ItemInst* inst = m_inv.PopItem(MainPowerSource);
		if (inst) {
			if (CheckLoreConflict(inst->GetItem())) {
				mlog(INVENTORY__ERROR, "Lore Duplication Error: Deleting %s from slot %i", inst->GetItem()->Name, slot_id);
				database.SaveInventory(character_id, nullptr, MainPowerSource);
			}
			else {
				m_inv.PutItem(MainPowerSource, *inst);
			}
			safe_delete(inst);
		}
	}

	// containers
	for(slot_id = EmuConstants::GENERAL_BAGS_BEGIN; slot_id <= EmuConstants::CURSOR_BAG_END; slot_id++) {
		ItemInst* inst = m_inv.PopItem(slot_id);
		if(inst) {
			if(CheckLoreConflict(inst->GetItem())) {
				mlog(INVENTORY__ERROR, "Lore Duplication Error: Deleting %s from slot %i", inst->GetItem()->Name, slot_id);
				database.SaveInventory(character_id, nullptr, slot_id);
			}
			else {
				m_inv.PutItem(slot_id, *inst);
			}
			safe_delete(inst);
		}
	}

	// bank
	for(slot_id = EmuConstants::BANK_BEGIN; slot_id <= EmuConstants::BANK_END; slot_id++) {
		ItemInst* inst = m_inv.PopItem(slot_id);
		if(inst) {
			if(CheckLoreConflict(inst->GetItem())) {
				mlog(INVENTORY__ERROR, "Lore Duplication Error: Deleting %s from slot %i", inst->GetItem()->Name, slot_id);
				database.SaveInventory(character_id, nullptr, slot_id);
			}
			else {
				m_inv.PutItem(slot_id, *inst);
			}
			safe_delete(inst);
		}
	}

	// bank containers
	for(slot_id = EmuConstants::BANK_BAGS_BEGIN; slot_id <= EmuConstants::BANK_BAGS_END; slot_id++) {
		ItemInst* inst = m_inv.PopItem(slot_id);
		if(inst) {
			if(CheckLoreConflict(inst->GetItem())) {
				mlog(INVENTORY__ERROR, "Lore Duplication Error: Deleting %s from slot %i", inst->GetItem()->Name, slot_id);
				database.SaveInventory(character_id, nullptr, slot_id);
			}
			else {
				m_inv.PutItem(slot_id, *inst);
			}
			safe_delete(inst);
		}
	}

	// Shared Bank and Shared Bank Containers are not checked due to their allowing duplicate lore items -U

	// cursor & limbo
	if (!m_inv.CursorEmpty()) {
		std::list<ItemInst*> local;
		ItemInst* inst = nullptr;

		while (!m_inv.CursorEmpty()) {
			inst = m_inv.PopItem(MainCursor);
			if (inst)
				local.push_back(inst);
		}

		std::list<ItemInst*>::iterator iter = local.begin();
		while (iter != local.end()) {
			inst = *iter;
			// probably needs a valid pointer check -U
			if (CheckLoreConflict(inst->GetItem())) {
				mlog(INVENTORY__ERROR, "Lore Duplication Error: Deleting %s from `Limbo`", inst->GetItem()->Name);
				safe_delete(*iter);
				iter = local.erase(iter);
			}
			else {
				++iter;
			}
		}

		iter = local.begin();
		while (iter != local.end()) {
			inst = *iter;
			if (!inst->GetItem()->LoreFlag ||
				((inst->GetItem()->LoreGroup == -1) && (m_inv.HasItem(inst->GetID(), 0, invWhereCursor) == INVALID_INDEX)) ||
				(inst->GetItem()->LoreGroup && ~inst->GetItem()->LoreGroup && (m_inv.HasItemByLoreGroup(inst->GetItem()->LoreGroup, invWhereCursor) == INVALID_INDEX))) {
				
				m_inv.PushCursor(**iter);
			}
			else {
				mlog(INVENTORY__ERROR, "Lore Duplication Error: Deleting %s from `Limbo`", inst->GetItem()->Name);
			}

			safe_delete(*iter);
			iter = local.erase(iter);
		}

		std::list<ItemInst*>::const_iterator s = m_inv.cursor_begin(), e = m_inv.cursor_end();
		database.SaveCursor(this->CharacterID(), s, e);
		local.clear();
	}
}

void Client::MoveSlotNotAllowed(bool client_update) {
	int16 slot_id = 0;

	// equipment
	for(slot_id = EmuConstants::EQUIPMENT_BEGIN; slot_id <= EmuConstants::EQUIPMENT_END; slot_id++) {
		if(m_inv[slot_id] && !m_inv[slot_id]->IsSlotAllowed(slot_id)) {
			ItemInst* inst = m_inv.PopItem(slot_id);
			bool is_arrow = (inst->GetItem()->ItemType == ItemTypeArrow) ? true : false;
			int16 free_slot_id = m_inv.FindFreeSlot(inst->IsType(ItemClassContainer), true, inst->GetItem()->Size, is_arrow);
			mlog(INVENTORY__ERROR, "Slot Assignment Error: Moving %s from slot %i to %i", inst->GetItem()->Name, slot_id, free_slot_id);
			PutItemInInventory(free_slot_id, *inst, client_update);
			database.SaveInventory(character_id, nullptr, slot_id);
			safe_delete(inst);
		}
	}

	// power source
	slot_id = MainPowerSource;
	if(m_inv[slot_id] && !m_inv[slot_id]->IsSlotAllowed(slot_id)) {
		ItemInst* inst = m_inv.PopItem(slot_id);
		bool is_arrow = (inst->GetItem()->ItemType == ItemTypeArrow) ? true : false;
		int16 free_slot_id = m_inv.FindFreeSlot(inst->IsType(ItemClassContainer), true, inst->GetItem()->Size, is_arrow);
		mlog(INVENTORY__ERROR, "Slot Assignment Error: Moving %s from slot %i to %i", inst->GetItem()->Name, slot_id, free_slot_id);
		PutItemInInventory(free_slot_id, *inst, (GetClientVersion() >= EQClientSoF) ? client_update : false);
		database.SaveInventory(character_id, nullptr, slot_id);
		safe_delete(inst);
	}

	// No need to check inventory, cursor, bank or shared bank since they allow max item size and containers -U
	// Code can be added to check item size vs. container size, but it is left to attrition for now.
}

// these functions operate with a material slot, which is from 0 to 8
uint32 Client::GetEquipment(uint8 material_slot) const
{
	int16 invslot;
	const ItemInst *item;

	if(material_slot > EmuConstants::MATERIAL_END)
	{
		return 0;
	}

	invslot = Inventory::CalcSlotFromMaterial(material_slot);
	if (invslot == INVALID_INDEX)
	{
		return 0;
	}

	item = m_inv.GetItem(invslot);

	if(item && item->GetItem())
	{
		return item->GetItem()->ID;
	}

	return 0;
}

/*
int32 Client::GetEquipmentMaterial(uint8 material_slot)
{
	const Item_Struct *item;

	item = database.GetItem(GetEquipment(material_slot));
	if(item != 0)
	{
		return item->Material;
	}

	return 0;
}
*/

uint32 Client::GetEquipmentColor(uint8 material_slot) const
{
	const Item_Struct *item;

	if(material_slot > EmuConstants::MATERIAL_END)
	{
		return 0;
	}

	item = database.GetItem(GetEquipment(material_slot));
	if(item != 0)
	{
		return m_pp.item_tint[material_slot].rgb.use_tint ?
			m_pp.item_tint[material_slot].color :
			item->Color;
	}

	return 0;
}

// Send an item packet (including all subitems of the item)
void Client::SendItemPacket(int16 slot_id, const ItemInst* inst, ItemPacketType packet_type)
{
	if (!inst)
		return;

	// Serialize item into |-delimited string
	std::string packet = inst->Serialize(slot_id);

	EmuOpcode opcode = OP_Unknown;
	EQApplicationPacket* outapp = nullptr;
	ItemPacket_Struct* itempacket = nullptr;

	// Construct packet
	opcode = (packet_type==ItemPacketViewLink) ? OP_ItemLinkResponse : OP_ItemPacket;
	outapp = new EQApplicationPacket(opcode, packet.length()+sizeof(ItemPacket_Struct));
	itempacket = (ItemPacket_Struct*)outapp->pBuffer;
	memcpy(itempacket->SerializedItem, packet.c_str(), packet.length());
	itempacket->PacketType = packet_type;

#if EQDEBUG >= 9
		DumpPacket(outapp);
#endif
	FastQueuePacket(&outapp);
}

EQApplicationPacket* Client::ReturnItemPacket(int16 slot_id, const ItemInst* inst, ItemPacketType packet_type)
{
	if (!inst)
		return 0;

	// Serialize item into |-delimited string
	std::string packet = inst->Serialize(slot_id);

	EmuOpcode opcode = OP_Unknown;
	EQApplicationPacket* outapp = nullptr;
	BulkItemPacket_Struct* itempacket = nullptr;

	// Construct packet
	opcode = OP_ItemPacket;
	outapp = new EQApplicationPacket(opcode, packet.length()+1);
	itempacket = (BulkItemPacket_Struct*)outapp->pBuffer;
	memcpy(itempacket->SerializedItem, packet.c_str(), packet.length());

#if EQDEBUG >= 9
		DumpPacket(outapp);
#endif

	return outapp;
}

static int16 BandolierSlotToWeaponSlot(int BandolierSlot) {

	switch(BandolierSlot) {
		case bandolierMainHand:
			return MainPrimary;
		case bandolierOffHand:
			return MainSecondary;
		case bandolierRange:
			return MainRange;
		default:
			return MainAmmo;
	}
}

void Client::CreateBandolier(const EQApplicationPacket *app) {

	// Store bandolier set with the number and name passed by the client, along with the items that are currently
	// in the players weapon slots.

	BandolierCreate_Struct *bs = (BandolierCreate_Struct*)app->pBuffer;

	_log(INVENTORY__BANDOLIER, "Char: %s Creating Bandolier Set %i, Set Name: %s", GetName(), bs->number, bs->name);
	strcpy(m_pp.bandoliers[bs->number].name, bs->name);

	const ItemInst* InvItem = nullptr; 
	const Item_Struct *BaseItem = nullptr; 
	int16 WeaponSlot;

	for(int BandolierSlot = bandolierMainHand; BandolierSlot <= bandolierAmmo; BandolierSlot++) {
		WeaponSlot = BandolierSlotToWeaponSlot(BandolierSlot);
		InvItem = GetInv()[WeaponSlot];
		if(InvItem) {
			BaseItem = InvItem->GetItem();
			_log(INVENTORY__BANDOLIER, "Char: %s adding item %s to slot %i", GetName(),BaseItem->Name, WeaponSlot);
			m_pp.bandoliers[bs->number].items[BandolierSlot].item_id = BaseItem->ID;
			m_pp.bandoliers[bs->number].items[BandolierSlot].icon = BaseItem->Icon;
			database.SaveCharacterBandolier(this->CharacterID(), bs->number, BandolierSlot, m_pp.bandoliers[bs->number].items[BandolierSlot].item_id, m_pp.bandoliers[bs->number].items[BandolierSlot].icon, bs->name);
		}
		else {
			_log(INVENTORY__BANDOLIER, "Char: %s no item in slot %i", GetName(), WeaponSlot);
			m_pp.bandoliers[bs->number].items[BandolierSlot].item_id = 0;
			m_pp.bandoliers[bs->number].items[BandolierSlot].icon = 0;
		}
	}
}

void Client::RemoveBandolier(const EQApplicationPacket *app) {
	BandolierDelete_Struct *bds = (BandolierDelete_Struct*)app->pBuffer;
	_log(INVENTORY__BANDOLIER, "Char: %s removing set", GetName(), bds->number);
	memset(m_pp.bandoliers[bds->number].name, 0, 32);
	for(int i = bandolierMainHand; i <= bandolierAmmo; i++) {
		m_pp.bandoliers[bds->number].items[i].item_id = 0;
		m_pp.bandoliers[bds->number].items[i].icon = 0; 
	}
	database.DeleteCharacterBandolier(this->CharacterID(), bds->number);
}

void Client::SetBandolier(const EQApplicationPacket *app) {

	// Swap the weapons in the given bandolier set into the character's weapon slots and return
	// any items currently in the weapon slots to inventory.

	BandolierSet_Struct *bss = (BandolierSet_Struct*)app->pBuffer;
	_log(INVENTORY__BANDOLIER, "Char: %s activating set %i", GetName(), bss->number);
	int16 slot;
	int16 WeaponSlot;
	ItemInst *BandolierItems[4]; // Temporary holding area for the weapons we pull out of their inventory

	// First we pull the items for this bandolier set out of their inventory, this makes space to put the
	// currently equipped items back.
	for(int BandolierSlot = bandolierMainHand; BandolierSlot <= bandolierAmmo; BandolierSlot++) {
		// If this bandolier set has an item in this position
		if(m_pp.bandoliers[bss->number].items[BandolierSlot].item_id) {
			WeaponSlot = BandolierSlotToWeaponSlot(BandolierSlot);

			// Check if the player has the item specified in the bandolier set on them.
			//
			slot = m_inv.HasItem(m_pp.bandoliers[bss->number].items[BandolierSlot].item_id, 1,
							invWhereWorn|invWherePersonal);

			// removed 'invWhereCursor' argument from above and implemented slots 30, 331-340 checks here
			if (slot == INVALID_INDEX) {
				if (m_inv.GetItem(MainCursor)) {
					if (m_inv.GetItem(MainCursor)->GetItem()->ID == m_pp.bandoliers[bss->number].items[BandolierSlot].item_id &&
						m_inv.GetItem(MainCursor)->GetCharges() >= 1) // '> 0' the same, but this matches Inventory::_HasItem conditional check
						slot = MainCursor;
					else if (m_inv.GetItem(MainCursor)->GetItem()->ItemClass == 1) {
						for(int16 CursorBagSlot = EmuConstants::CURSOR_BAG_BEGIN; CursorBagSlot <= EmuConstants::CURSOR_BAG_END; CursorBagSlot++) {
							if (m_inv.GetItem(CursorBagSlot)) {
								if (m_inv.GetItem(CursorBagSlot)->GetItem()->ID == m_pp.bandoliers[bss->number].items[BandolierSlot].item_id &&
									m_inv.GetItem(CursorBagSlot)->GetCharges() >= 1) { // ditto
										slot = CursorBagSlot;
										break;
								}
							}
						}
					}
				}
			}

			// if the player has this item in their inventory,
			if (slot != INVALID_INDEX) {
				// Pull the item out of the inventory
				BandolierItems[BandolierSlot] = m_inv.PopItem(slot);
				// If ammo with charges, only take one charge out to put in the range slot, that is what
				// the client does.

				if(((BandolierSlot == bandolierAmmo) || (BandolierSlot == bandolierRange)) &&
					BandolierItems[BandolierSlot] && BandolierItems[BandolierSlot]->IsStackable()){
					int Charges = BandolierItems[BandolierSlot]->GetCharges();
					// If there is more than one charge
					if(Charges > 1) {
						BandolierItems[BandolierSlot]->SetCharges(Charges-1);
						// Take one charge out and put the rest back
						m_inv.PutItem(slot, *BandolierItems[BandolierSlot]);
						database.SaveInventory(character_id, BandolierItems[BandolierSlot], slot);
						BandolierItems[BandolierSlot]->SetCharges(1);
					}
					else	// Remove the item from the inventory
						database.SaveInventory(character_id, 0, slot);
				}
				else	// Remove the item from the inventory
					database.SaveInventory(character_id, 0, slot);
			}
			else { // The player doesn't have the required weapon with them.
				BandolierItems[BandolierSlot] = 0;
				if (slot == INVALID_INDEX) {
					_log(INVENTORY__BANDOLIER, "Character does not have required bandolier item for slot %i", WeaponSlot);
					ItemInst *InvItem = m_inv.PopItem(WeaponSlot);
					if(InvItem) {
						// If there was an item in that weapon slot, put it in the inventory
						_log(INVENTORY__BANDOLIER, "returning item %s in weapon slot %i to inventory",
										InvItem->GetItem()->Name, WeaponSlot);
						if(MoveItemToInventory(InvItem))
							database.SaveInventory(character_id, 0, WeaponSlot);
						else
							_log(INVENTORY__BANDOLIER, "Char: %s, ERROR returning %s to inventory", GetName(),
								InvItem->GetItem()->Name);
						safe_delete(InvItem);
					}

				}
			}
		}
	}

	// Now we move the required weapons into the character weapon slots, and return any items we are replacing
	// back to inventory.
	//
	for(int BandolierSlot = bandolierMainHand; BandolierSlot <= bandolierAmmo; BandolierSlot++) {

		// Find the inventory slot corresponding to this bandolier slot

		WeaponSlot = BandolierSlotToWeaponSlot(BandolierSlot);

		// if there is an item in this Bandolier slot ?
		if(m_pp.bandoliers[bss->number].items[BandolierSlot].item_id) {
			// if the player has this item in their inventory, and it is not already where it needs to be
			if(BandolierItems[BandolierSlot]) {
				// Pull the item that we are going to replace
				ItemInst *InvItem = m_inv.PopItem(WeaponSlot);
				// Put the item specified in the bandolier where it needs to be
				m_inv.PutItem(WeaponSlot, *BandolierItems[BandolierSlot]);

				safe_delete(BandolierItems[BandolierSlot]);
				// Update the database, save the item now in the weapon slot
				database.SaveInventory(character_id, m_inv.GetItem(WeaponSlot), WeaponSlot);

				if(InvItem) {
					// If there was already an item in that weapon slot that we replaced, find a place to put it
					if(!MoveItemToInventory(InvItem))
						_log(INVENTORY__BANDOLIER, "Char: %s, ERROR returning %s to inventory", GetName(),
							InvItem->GetItem()->Name);
					safe_delete(InvItem);
				}
			}
		}
		else {
			// This bandolier set has no item for this slot, so take whatever is in the weapon slot and
			// put it in the player's inventory.
			ItemInst *InvItem = m_inv.PopItem(WeaponSlot);
			if(InvItem) {
				_log(INVENTORY__BANDOLIER, "Bandolier has no item for slot %i, returning item %s to inventory",
								WeaponSlot, InvItem->GetItem()->Name);
				// If there was an item in that weapon slot, put it in the inventory
				if(MoveItemToInventory(InvItem))
					database.SaveInventory(character_id, 0, WeaponSlot);
				else
					_log(INVENTORY__BANDOLIER, "Char: %s, ERROR returning %s to inventory", GetName(),
						InvItem->GetItem()->Name);
				safe_delete(InvItem);
			}
		}
	}
	// finally, recalculate any stat bonuses from the item change
	CalcBonuses();
}

bool Client::MoveItemToInventory(ItemInst *ItemToReturn, bool UpdateClient) {

	// This is a support function for Client::SetBandolier, however it can be used anywhere it's functionality is required.
	//
	// When the client moves items around as Bandolier sets are activated, it does not send details to the
	// server of what item it has moved to which slot. It assumes the server knows what it will do.
	//
	// The standard EQEmu auto inventory routines do not behave as the client does when manipulating bandoliers.
	// The client will look in each main inventory slot. If it finds a bag in a slot, it will then look inside
	// the bag for a free slot.
	//
	// This differs from the standard EQEmu method of looking in all 8 inventory slots first to find an empty slot, and
	// then going back and looking in bags. There are also other differences related to how it moves stackable items back
	// to inventory.
	//
	// Rather than alter the current auto inventory behaviour, just in case something
	// depends on current behaviour, this routine operates the same as the client when moving items back to inventory when
	// swapping bandolier sets.

	if(!ItemToReturn) return false;

	_log(INVENTORY__SLOTS,"Char: %s Returning %s to inventory", GetName(), ItemToReturn->GetItem()->Name);

	uint32 ItemID = ItemToReturn->GetItem()->ID;

	// If the item is stackable (ammo in range slot), try stacking it with other items of the same type
	//
	if(ItemToReturn->IsStackable()) {

		for (int16 i = EmuConstants::GENERAL_BEGIN; i <= MainCursor; i++) { // changed slot max to 30 from 29. client will stack into slot 30 (bags too) before moving.

			ItemInst* InvItem = m_inv.GetItem(i);

			if(InvItem && (InvItem->GetItem()->ID == ItemID) && (InvItem->GetCharges() < InvItem->GetItem()->StackSize)) {

				int ChargeSlotsLeft = InvItem->GetItem()->StackSize - InvItem->GetCharges();

				int ChargesToMove = ItemToReturn->GetCharges() < ChargeSlotsLeft ? ItemToReturn->GetCharges() :
													ChargeSlotsLeft;

				InvItem->SetCharges(InvItem->GetCharges() + ChargesToMove);

				if(UpdateClient)
					SendItemPacket(i, InvItem, ItemPacketTrade);

				database.SaveInventory(character_id, m_inv.GetItem(i), i);

				ItemToReturn->SetCharges(ItemToReturn->GetCharges() - ChargesToMove);

				if(!ItemToReturn->GetCharges())
					return true;
			}
			// If there is a bag in this slot, look inside it.
			//
			if (InvItem && InvItem->IsType(ItemClassContainer)) {

				int16 BaseSlotID = Inventory::CalcSlotId(i, SUB_BEGIN);

				uint8 BagSize=InvItem->GetItem()->BagSlots;

				uint8 BagSlot;
				for (BagSlot = SUB_BEGIN; BagSlot < BagSize; BagSlot++) {
					InvItem = m_inv.GetItem(BaseSlotID + BagSlot);
					if (InvItem && (InvItem->GetItem()->ID == ItemID) &&
						(InvItem->GetCharges() < InvItem->GetItem()->StackSize)) {

						int ChargeSlotsLeft = InvItem->GetItem()->StackSize - InvItem->GetCharges();

						int ChargesToMove = ItemToReturn->GetCharges() < ChargeSlotsLeft
									? ItemToReturn->GetCharges() : ChargeSlotsLeft;

						InvItem->SetCharges(InvItem->GetCharges() + ChargesToMove);

						if(UpdateClient)
							SendItemPacket(BaseSlotID + BagSlot, m_inv.GetItem(BaseSlotID + BagSlot),
										ItemPacketTrade);

						database.SaveInventory(character_id, m_inv.GetItem(BaseSlotID + BagSlot),
										BaseSlotID + BagSlot);

						ItemToReturn->SetCharges(ItemToReturn->GetCharges() - ChargesToMove);

						if(!ItemToReturn->GetCharges())
							return true;
					}
				}
			}
		}
	}

	// We have tried stacking items, now just try and find an empty slot.

	for (int16 i = EmuConstants::GENERAL_BEGIN; i <= MainCursor; i++) { // changed slot max to 30 from 29. client will move into slot 30 (bags too) before pushing onto cursor.

		ItemInst* InvItem = m_inv.GetItem(i);

		if (!InvItem) {
			// Found available slot in personal inventory
			m_inv.PutItem(i, *ItemToReturn);

			if(UpdateClient)
				SendItemPacket(i, ItemToReturn, ItemPacketTrade);

			database.SaveInventory(character_id, m_inv.GetItem(i), i);

			_log(INVENTORY__SLOTS, "Char: %s Storing in main inventory slot %i", GetName(), i);

			return true;
		}
		if(InvItem->IsType(ItemClassContainer) && Inventory::CanItemFitInContainer(ItemToReturn->GetItem(), InvItem->GetItem())) {

			int16 BaseSlotID = Inventory::CalcSlotId(i, SUB_BEGIN);

			uint8 BagSize=InvItem->GetItem()->BagSlots;

			for (uint8 BagSlot = SUB_BEGIN; BagSlot < BagSize; BagSlot++) {

				InvItem = m_inv.GetItem(BaseSlotID + BagSlot);

				if (!InvItem) {
					// Found available slot within bag
					m_inv.PutItem(BaseSlotID + BagSlot, *ItemToReturn);

					if(UpdateClient)
						SendItemPacket(BaseSlotID + BagSlot, ItemToReturn, ItemPacketTrade);

					database.SaveInventory(character_id, m_inv.GetItem(BaseSlotID + BagSlot), BaseSlotID + BagSlot);

					_log(INVENTORY__SLOTS, "Char: %s Storing in bag slot %i", GetName(), BaseSlotID + BagSlot);

					return true;
				}
			}
		}
	}

	// Store on the cursor
	//
	_log(INVENTORY__SLOTS, "Char: %s No space, putting on the cursor", GetName());

	PushItemOnCursor(*ItemToReturn, UpdateClient);

	return true;
}

bool Client::InterrogateInventory(Client* requester, bool log, bool silent, bool allowtrip, bool& error, bool autolog)
{
	if (!requester)
		return false;

	std::map<int16, const ItemInst*> instmap;

	// build reference map
	for (int16 index = MAIN_BEGIN; index < EmuConstants::MAP_POSSESSIONS_SIZE; ++index)
		if (m_inv[index])
			instmap[index] = m_inv[index];
	for (int16 index = EmuConstants::TRIBUTE_BEGIN; index <= EmuConstants::TRIBUTE_END; ++index)
		if (m_inv[index])
			instmap[index] = m_inv[index];
	for (int16 index = EmuConstants::BANK_BEGIN; index <= EmuConstants::BANK_END; ++index)
		if (m_inv[index])
			instmap[index] = m_inv[index];
	for (int16 index = EmuConstants::SHARED_BANK_BEGIN; index <= EmuConstants::SHARED_BANK_END; ++index)
		if (m_inv[index])
			instmap[index] = m_inv[index];
	for (int16 index = EmuConstants::TRADE_BEGIN; index <= EmuConstants::TRADE_END; ++index)
		if (m_inv[index])
			instmap[index] = m_inv[index];

	if (Object* tsobject = GetTradeskillObject())
		for (int16 index = MAIN_BEGIN; index < EmuConstants::MAP_WORLD_SIZE; ++index)
			if (tsobject->GetItem(index))
				instmap[EmuConstants::WORLD_BEGIN + index] = tsobject->GetItem(index);

	int limbo = 0;
	for (iter_queue cursor_itr = m_inv.cursor_begin(); cursor_itr != m_inv.cursor_end(); ++cursor_itr, ++limbo) {
		if (cursor_itr == m_inv.cursor_begin()) // m_inv.cursor_begin() is referenced as MainCursor in MapPossessions above
			continue;

		instmap[8000 + limbo] = *cursor_itr;
	}

	if (m_inv[MainPowerSource])
		instmap[MainPowerSource] = m_inv[MainPowerSource];

	// call InterrogateInventory_ for error check
	for (std::map<int16, const ItemInst*>::iterator instmap_itr = instmap.begin(); (instmap_itr != instmap.end()) && (!error); ++instmap_itr)
		InterrogateInventory_(true, requester, instmap_itr->first, INVALID_INDEX, instmap_itr->second, nullptr, log, silent, error, 0);

	if (autolog && error && (!log))
		log = true;

	if (log)
		_log(INVENTORY__ERROR, "Client::InterrogateInventory() called for %s by %s with an error state of %s", GetName(), requester->GetName(), (error ? "TRUE" : "FALSE"));
	if (!silent)
		requester->Message(1, "--- Inventory Interrogation Report for %s (requested by: %s, error state: %s) ---", GetName(), requester->GetName(), (error ? "TRUE" : "FALSE"));

	// call InterrogateInventory_ for report
	for (std::map<int16, const ItemInst*>::iterator instmap_itr = instmap.begin(); (instmap_itr != instmap.end()); ++instmap_itr)
		InterrogateInventory_(false, requester, instmap_itr->first, INVALID_INDEX, instmap_itr->second, nullptr, log, silent, error, 0);

	if (error) {
		Message(13, "An error has been discovered in your inventory!");
		Message(13, "Do not log out, zone or re-arrange items until this");
		Message(13, "issue has been resolved or item loss may occur!");

		if (allowtrip)
			TripInterrogateInvState();
	}

	if (log) {
		_log(INVENTORY__ERROR, "Target interrogate inventory flag: %s", (GetInterrogateInvState() ? "TRUE" : "FALSE"));
		_log(INVENTORY__ERROR, "Client::InterrogateInventory() -- End");
	}
	if (!silent) {
		requester->Message(1, "Target interrogation flag: %s", (GetInterrogateInvState() ? "TRUE" : "FALSE"));
		requester->Message(1, "--- End of Interrogation Report ---");
	}

	instmap.clear();

	return true;
}

void Client::InterrogateInventory_(bool errorcheck, Client* requester, int16 head, int16 index, const ItemInst* inst, const ItemInst* parent, bool log, bool silent, bool &error, int depth)
{
	if (depth >= 10) {
		_log(INVENTORY__ERROR, "Client::InterrogateInventory_() - Recursion count has exceeded the maximum allowable (You have a REALLY BIG PROBLEM!!)");
		return;
	}

	if (errorcheck) {
		if (InterrogateInventory_error(head, index, inst, parent, depth)) {
			error = true;
		}
		else {
			if (inst)
				for (int16 sub = SUB_BEGIN; (sub < EmuConstants::ITEM_CONTAINER_SIZE) && (!error); ++sub) // treat any ItemInst as having the max internal slots available
					if (inst->GetItem(sub))
						InterrogateInventory_(true, requester, head, sub, inst->GetItem(sub), inst, log, silent, error, depth + 1);
		}
	}
	else {
		bool localerror = InterrogateInventory_error(head, index, inst, parent, depth);
		std::string i;
		std::string p;
		std::string e;

		if (inst) { i = StringFormat("%s (class: %u | augtype: %u)", inst->GetItem()->Name, inst->GetItem()->ItemClass, inst->GetItem()->AugType); }
		else { i = "NONE"; }
		if (parent) { p = StringFormat("%s (class: %u | augtype: %u), index: %i", parent->GetItem()->Name, parent->GetItem()->ItemClass, parent->GetItem()->AugType, index); }
		else { p = "NONE"; }
		if (localerror) { e = " [ERROR]"; }
		else { e = ""; }

		if (log)
			_log(INVENTORY__ERROR, "Head: %i, Depth: %i, Instance: %s, Parent: %s%s",
			head, depth, i.c_str(), p.c_str(), e.c_str());
		if (!silent)
			requester->Message(1, "%i:%i - inst: %s - parent: %s%s",
			head, depth, i.c_str(), p.c_str(), e.c_str());

		if (inst)
			for (int16 sub = SUB_BEGIN; (sub < EmuConstants::ITEM_CONTAINER_SIZE); ++sub)
				if (inst->GetItem(sub))
					InterrogateInventory_(false, requester, head, sub, inst->GetItem(sub), inst, log, silent, error, depth + 1);
	}

	return;
}

bool Client::InterrogateInventory_error(int16 head, int16 index, const ItemInst* inst, const ItemInst* parent, int depth)
{
	// very basic error checking - can be elaborated upon if more in-depth testing is needed...

	if (
		(head >= EmuConstants::EQUIPMENT_BEGIN && head <= EmuConstants::EQUIPMENT_END) ||
		(head >= EmuConstants::TRIBUTE_BEGIN && head <= EmuConstants::TRIBUTE_END) ||
		(head >= EmuConstants::WORLD_BEGIN && head <= EmuConstants::WORLD_END) ||
		(head >= 8000 && head <= 8101) ||
		(head == MainPowerSource)) {
		switch (depth)
		{
		case 0: // requirement: inst is extant
			if (!inst)
				return true;
			break;
		case 1: // requirement: parent is common and inst is augment
			if ((!parent) || (!inst))
				return true;
			if (!parent->IsType(ItemClassCommon))
				return true;
			if (index >= EmuConstants::ITEM_COMMON_SIZE)
				return true;
			break;
		default: // requirement: none (something bad happened...)
			return true;
		}
	}
	else if (
		(head >= EmuConstants::GENERAL_BEGIN && head <= EmuConstants::GENERAL_END) ||
		(head == MainCursor) ||
		(head >= EmuConstants::BANK_BEGIN && head <= EmuConstants::BANK_END) ||
		(head >= EmuConstants::SHARED_BANK_BEGIN && head <= EmuConstants::SHARED_BANK_END) ||
		(head >= EmuConstants::TRADE_BEGIN && head <= EmuConstants::TRADE_END)) {
		switch (depth)
		{
		case 0: // requirement: inst is extant
			if (!inst)
				return true;
			break;
		case 1: // requirement: parent is common and inst is augment ..or.. parent is container and inst is extant
			if ((!parent) || (!inst))
				return true;
			if (parent->IsType(ItemClassContainer))
				break;
			if (parent->IsType(ItemClassBook))
				return true;
			if (parent->IsType(ItemClassCommon)) {
				if (!(inst->GetItem()->AugType > 0))
					return true;
				if (index >= EmuConstants::ITEM_COMMON_SIZE)
					return true;
			}
			break;
		case 2: // requirement: parent is common and inst is augment
			if ((!parent) || (!inst))
				return true;
			if (parent->IsType(ItemClassContainer))
				return true;
			if (parent->IsType(ItemClassBook))
				return true;
			if (parent->IsType(ItemClassCommon)) {
				if (!(inst->GetItem()->AugType > 0))
					return true;
				if (index >= EmuConstants::ITEM_COMMON_SIZE)
					return true;
			}
			break;
		default: // requirement: none (something bad happened again...)
			return true;
		}
	}
	else {
		return true;
	}

	return false;
}

void Inventory::SetCustomItemData(uint32 character_id, int16 slot_id, std::string identifier, std::string value) {
	ItemInst *inst = GetItem(slot_id);
	if(inst) {
		inst->SetCustomData(identifier, value);
		database.SaveInventory(character_id, inst, slot_id);
	}
}

void Inventory::SetCustomItemData(uint32 character_id, int16 slot_id, std::string identifier, int value) {
	ItemInst *inst = GetItem(slot_id);
	if(inst) {
		inst->SetCustomData(identifier, value);
		database.SaveInventory(character_id, inst, slot_id);
	}
}

void Inventory::SetCustomItemData(uint32 character_id, int16 slot_id, std::string identifier, float value) {
	ItemInst *inst = GetItem(slot_id);
	if(inst) {
		inst->SetCustomData(identifier, value);
		database.SaveInventory(character_id, inst, slot_id);
	}
}

void Inventory::SetCustomItemData(uint32 character_id, int16 slot_id, std::string identifier, bool value) {
	ItemInst *inst = GetItem(slot_id);
	if(inst) {
		inst->SetCustomData(identifier, value);
		database.SaveInventory(character_id, inst, slot_id);
	}
}

std::string Inventory::GetCustomItemData(int16 slot_id, std::string identifier) {
	ItemInst *inst = GetItem(slot_id);
	if(inst) {
		return inst->GetCustomData(identifier);
	}
	return "";
}
