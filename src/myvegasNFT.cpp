#include <eosiolib/eosio.hpp>

using namespace eosio;

class[[eosio::contract]] myvegasNFT : public eosio::contract{

public:
  using contract::contract;

  myvegasNFT(name receiver, name code,  datastream<const char*> ds) : contract(receiver, code, ds) {}

  void create(name issuer, uint64_t token_id, uint64_t token_status) {

	  require_auth(_self);

	  stats statstable(_self, _self.value);
	  auto existing = statstable.find(token_id);
	  eosio_assert(existing == statstable.end(), "token with token_id already exists");

	  statstable.emplace(_self, [&](auto& s) {
		  s.token_id = token_id;
		  s.token_status= token_status;
		  s.issuer = issuer;
		  s.owner = issuer;
	  });
  }

  /* we dont actually need a issue function? we create and transfer
  void issue(name to, uint64_t token_id, string memo)
  {

	  stats statstable(_self, _self.value);
	  auto existing = statstable.find(token_id);
	  eosio_assert(existing != statstable.end(), "token with token_id does not exist, create token before issue");

	  const auto& st = *existing;
	  require_auth(st.issuer);

	  eosio_assert(st.owner == st.issuer, "token already issued");


	  add_balance(st.issuer, quantity, st.issuer); //???

	  if (to != st.issuer) {
		  SEND_INLINE_ACTION(*this, transfer, { {st.issuer, "active"_n} },
			  { st.issuer, to, quantity, memo }
		  );
	  }
  }

  */

  void token::retire(uin64_t token_id, string memo)
  {
	  
	  stats statstable(_self,_self.value);
	  auto existing = statstable.find(token_id);
	  eosio_assert(existing != statstable.end(), "token with token_id does not exist");
	  const auto& st = *existing;

	  require_auth(st.issuer);


	  statstable.modify(st, _self, [&](auto& s) {
		  s.token_status = 0;
	  });
  }

  void token::transfer(name    from,
	  name    to,
	  uint64_t token_id,
	  string  memo)
  {
	  eosio_assert(from != to, "cannot transfer to self");
	  require_auth(from);
	  eosio_assert(is_account(to), "to account does not exist");

	  stats statstable(_self, _self.value);
	  auto existing = statstable.find(token_id);
	  eosio_assert(existing != statstable.end(), "token with token_id does not exist");

	  require_recipient(from);
	  require_recipient(to);

	  eosio_assert(memo.size() <= 256, "memo has more than 256 bytes");

	  auto payer = has_auth(to) ? to : from;

	  account_addtoken(from, token_id);
	  account_removetoken(to, token_id, payer);
  }

  void account_removetoken(name owner, uint64_t token_id) {
	  accounts from_acnts(_self, _self.value);

	  const auto& from = from_acnts.get(token_id, "no token object found");

	  from_acnts.erase(from);
  }

  void account_addtoken(name owner, uint64_t token_id, name ram_payer)
  {
	  accounts to_acnts(_self, _self.value);
	  to_acnts.emplace(ram_payer, [&](auto& a) {
			 a.token_id = token_id;
		});

	  stats statstable(_self, _self.value);
	  auto existing = statstable.find(token_id);
	  eosio_assert(existing != statstable.end(), "token with token_id does not exist");
	  const auto& st = *existing;
	  statstable.modify(st, ram_payer, [&](auto& s) {
		  s.owner = owner;
	  });
	  //same_payer?

  }

private:
	struct[[eosio::table]] movNFT_stats{

		uint64_t token_id;
		uint64_t token_status;
		name     issuer;
		name     owner;

	uint64_t primary_key()const { return token_id; }

	};
	
	struct[[eosio::table]] account{
		
		uint64_t token_id;

	uint64_t primary_key()const { return token_id; }
	};

	typedef eosio::multi_index< "accounts"_n, account > accounts;

	typedef eosio::multi_index< "stat"_n, movNFT_stats > stats;
};

EOSIO_DISPATCH(eosio::token, (create)(transfer)(retire))